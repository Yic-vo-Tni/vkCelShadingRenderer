
#if _WIN32
#include <Windows.h>
#include <shellapi.h>
#undef min
#undef max
#endif

#include "vkPmx/vk_saba2.h"

struct Model
{
	std::shared_ptr<saba::MMDModel>		m_mmdModel;
	std::unique_ptr<saba::VMDAnimation>	m_vmdAnim;

    std::unique_ptr<yic::genericBufferManager> mIndexBuffer{};
	vk::IndexType	m_indexType{};

	bool Setup(AppContext& appContext);
	bool SetupVertexBuffer(AppContext& appContext);
	bool SetupDescriptorSet(AppContext & appContext);
	bool SetupCommandBuffer();

	void UpdateAnimation(const AppContext& appContext);
	void Update(AppContext& appContext);
	void Draw(AppContext& appContext);

	[[nodiscard]] vk::CommandBuffer GetCommandBuffer(uint32_t imageIndex) const;

	struct ModelResource
	{
        std::unique_ptr<yic::genericBufferManager> mVertexBuffer{};
        std::unique_ptr<yic::genericBufferManager> mUniformBuffer{};

		// MMD Shader
		uint32_t	m_mmdVSUBOffset{};
	};

	struct Resource
	{
		ModelResource					m_modelResource;
		std::vector<uint32_t >	mMmdFragUniformOffset{};
	};

	Resource				m_resource;
	vk::DescriptorPool		m_descPool;

	std::vector<vk::CommandBuffer>	m_cmdBuffers;

    /////////////////////////

	std::vector<genericTexManagerUptr>	mMmdMaterials;

    uint32_t mImageCount{};
    vk::Device mDevice;
    vk::PhysicalDevice mPhysicalDevice;
    vk::CommandPool mCommandPool;
    std::vector<vk::DescriptorSetLayout> mSetLayouts;
};

/*
	Model
*/
bool Model::Setup(AppContext& appContext)
{
	vk::Result ret;
	mDevice = appContext.m_device;
    mPhysicalDevice = appContext.m_gpu;
    mImageCount = appContext.m_imageCount;
    mSetLayouts = appContext.mMmdDescriptor.getDescriptorSetLayouts();
    mCommandPool = appContext.m_commandPool;

	auto swapImageCount = uint32_t(appContext.m_swapchainImageResouces.size());

	size_t matCount = m_mmdModel->GetMaterialCount();

    appContext.mMmdDescriptor.increaseMaxSets(matCount - 1);
    appContext.mMmdDescriptor.createDescriptorPool();
    m_descPool = appContext.mMmdDescriptor.getDescriptorPool();

	mMmdMaterials.resize(matCount);
    auto cmd = appContext.mvkContext.createTempCmdBuf();
	for (size_t i = 0; i < matCount; i++){
		const auto materials = m_mmdModel->GetMaterials();
		const auto& mmdMat = materials[i];
        mMmdMaterials[i] = std::make_unique<yic::vkImage>(mmdMat.m_texture);
	}
    appContext.mvkContext.submitTempCmdBuf(cmd);

    for(size_t i = 0; i < matCount; i++){
        appContext.mMmdDescriptor.pushBackDesSets();
    }

	SetupVertexBuffer(appContext);
	SetupDescriptorSet(appContext);
	SetupCommandBuffer();

	return true;
}

bool Model::SetupVertexBuffer(AppContext & appContext)
{
	vk::Result ret;

	// Vertex Buffer
	{
		//auto& res = m_resources[i];
		auto& res = m_resource;
		auto& modelRes = res.m_modelResource;
		//auto& vb = modelRes.m_vertexBuffer;
        auto& vb = modelRes.mVertexBuffer;

		// Create buffer
		auto vbMemSize = uint32_t(sizeof(Vertex) * m_mmdModel->GetVertexCount());

        vb = std::make_unique<yic::genericBufferManager>(vbMemSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
	}

	// Index Buffer
	{
		// Create buffer
		auto ibMemSize = uint32_t(m_mmdModel->GetIndexElementSize() * m_mmdModel->GetIndexCount());
        mIndexBuffer = std::make_unique<yic::genericBufferManager>(ibMemSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);

		StagingBuffer* indicesStagingBuffer;
		if (!StagingBuffer::getStagingBuffer(appContext,ibMemSize, &indicesStagingBuffer))
		{
			std::cout << "Failed to get Staging Buffer.\n";
			return false;
		}

		void* mapMem;
		ret = mDevice.mapMemory(indicesStagingBuffer->m_memory, 0, ibMemSize, vk::MemoryMapFlagBits(0), &mapMem);
		if (vk::Result::eSuccess != ret)
		{
			std::cout << "Failed to map memory.\n";
			return false;
		}
		std::memcpy(mapMem, m_mmdModel->GetIndices(), ibMemSize);
        mDevice.unmapMemory(indicesStagingBuffer->m_memory);

		if (!indicesStagingBuffer->CopyBuffer(mIndexBuffer->getBuffer(), ibMemSize))
		{
			std::cout << "Failed to copy buffer.\n";
			return false;
		}
		indicesStagingBuffer->Wait();

		if (m_mmdModel->GetIndexElementSize() == 2)
		{
			m_indexType = vk::IndexType::eUint16;
		}
		else if (m_mmdModel->GetIndexElementSize() == 4)
		{
			m_indexType = vk::IndexType::eUint32;
		}
		else
		{
			std::cout << "Unknown index size.[" << m_mmdModel->GetIndexElementSize() << "]\n";
			return false;
		}
	}

	return true;
}

bool Model::SetupDescriptorSet(AppContext & appContext)
{
	auto gpuProp = mPhysicalDevice.getProperties();
	uint32_t ubAlign = uint32_t(gpuProp.limits.minUniformBufferOffsetAlignment);
	{
		uint32_t ubOffset = 0;

		//auto& res = m_resource;
		auto& modelRes = m_resource.m_modelResource;

		// MMDVertxShaderUB
		modelRes.m_mmdVSUBOffset = ubOffset;
		ubOffset += sizeof(MMDVertxShaderUB);
		ubOffset = (ubOffset + ubAlign) - ((ubOffset + ubAlign) % ubAlign);

		size_t matCount = m_mmdModel->GetMaterialCount();
        m_resource.mMmdFragUniformOffset.resize(matCount);
		for (size_t matIdx = 0; matIdx < matCount; matIdx++)
		{
			auto& matRes = m_resource.mMmdFragUniformOffset[matIdx];



			// MMDFragmentShaderUB
			matRes = ubOffset;
			ubOffset += sizeof(MMDFragmentShaderUB);
			ubOffset = (ubOffset + ubAlign) - ((ubOffset + ubAlign) % ubAlign);
		}

		// Create Uniform Buffer
		auto& ub = modelRes.mUniformBuffer;
		auto ubMemSize = ubOffset;

        ub = std::make_unique<yic::genericBufferManager>(ubMemSize, vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst);
	}

	// MMDVertxShaderUB
	{
		auto& res = m_resource;
		auto& modelRes = res.m_modelResource;

		size_t matCount = m_mmdModel->GetMaterialCount();

		for (size_t matIdx = 0; matIdx < matCount; matIdx++)
		{
			auto& matRes = res.mMmdFragUniformOffset[matIdx];

            appContext.mMmdDescriptor.updateDescriptorSet({
                vk::DescriptorBufferInfo{modelRes.mUniformBuffer->getBuffer(), modelRes.m_mmdVSUBOffset, sizeof(MMDVertxShaderUB)},
                vk::DescriptorBufferInfo{modelRes.mUniformBuffer->getBuffer(), matRes, sizeof(MMDFragmentShaderUB)},
                vk::DescriptorImageInfo{mMmdMaterials[matIdx]->getSampler(),mMmdMaterials[matIdx]->getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal},
            });
		}

	}



	return true;
}

bool Model::SetupCommandBuffer()
{
	vk::Result ret;

	std::vector<vk::CommandBuffer> cmdBuffers(mImageCount);

	auto cmdBufInfo = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(mImageCount)
		.setCommandPool(mCommandPool)
		.setLevel(vk::CommandBufferLevel::eSecondary);
	ret = mDevice.allocateCommandBuffers(&cmdBufInfo, cmdBuffers.data());
	if (vk::Result::eSuccess != ret)
	{
		std::cout << "Failed to allocate Models Command Buffer.\n";
		return false;
	}
	m_cmdBuffers = std::move(cmdBuffers);

	return true;
}


void Model::UpdateAnimation(const AppContext& appContext)
{
	m_mmdModel->BeginAnimation();
	m_mmdModel->UpdateAllAnimation(m_vmdAnim.get(), appContext.m_animTime * 30.0f, appContext.m_elapsed);
	m_mmdModel->EndAnimation();
}

void Model::Update(AppContext& appContext)
{
	vk::Result ret;

	auto& res = m_resource;

	auto device = appContext.m_device;

	size_t vtxCount = m_mmdModel->GetVertexCount();
	m_mmdModel->Update();
	const glm::vec3* position = m_mmdModel->GetUpdatePositions();
	const glm::vec3* normal = m_mmdModel->GetUpdateNormals();
	const glm::vec2* uv = m_mmdModel->GetUpdateUVs();

	// Update vertices

	auto memSize = vk::DeviceSize(sizeof(Vertex) * vtxCount);
	StagingBuffer* vbStBuf;
	if (!StagingBuffer::getStagingBuffer(appContext, memSize, &vbStBuf))
	{
		std::cout << "Failed to get Staging Buffer.\n";
		return;
	}

	void* vbStMem;
	ret = device.mapMemory(vbStBuf->m_memory, 0, memSize, vk::MemoryMapFlags(), &vbStMem);
	if (vk::Result::eSuccess != ret)
	{
		std::cout << "Failed to map memory.\n";
		return;
	}
	auto v = static_cast<Vertex*>(vbStMem);
	for (size_t i = 0; i < vtxCount; i++)
	{
		v->m_position = *position;
		v->m_normal = *normal;
		v->m_uv = *uv;
		v++;
		position++;
		normal++;
		uv++;
	}
	device.unmapMemory(vbStBuf->m_memory);

	if (!vbStBuf->CopyBuffer(res.m_modelResource.mVertexBuffer->getBuffer(), memSize))
	{
		std::cout << "Failed to copy buffer.\n";
		return;
	}


	// Update uniform buffer
	auto ubMemSize = res.m_modelResource.mUniformBuffer->getDeviceSize();
	StagingBuffer* ubStBuf;
	if (!StagingBuffer::getStagingBuffer(appContext, ubMemSize, &ubStBuf))
	{
		std::cout << "Failed to get Staging Buffer.\n";
		return;
	}
	uint8_t* ubPtr;
	ret = device.mapMemory(ubStBuf->m_memory, 0, ubMemSize, vk::MemoryMapFlags(), (void**)&ubPtr);
	if (vk::Result::eSuccess != ret)
	{
		std::cout << "Failed to map memory.\n";
		return;
	}

	// Write Model uniform buffer
	auto& modelRes = res.m_modelResource;
	{
		const auto world = glm::mat4(1.0f);
		const auto& view = appContext.m_viewMat;
		const auto& proj = appContext.m_projMat;
		glm::mat4 vkMat = glm::mat4(
			1.0f,  0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f,  0.0f, 0.5f, 0.0f,
			0.0f,  0.0f, 0.5f, 1.0f
		);
		auto wv = view;
		auto wvp = vkMat * proj * view;
		auto mmdVSUB = reinterpret_cast<MMDVertxShaderUB*>(ubPtr + res.m_modelResource.m_mmdVSUBOffset);
		mmdVSUB->m_wv = wv;
		mmdVSUB->m_wvp = wvp;
	}

	// Write Material uniform buffer;
	{
		auto matCount = m_mmdModel->GetMaterialCount();
		for (size_t i = 0; i < matCount; i++)
		{
			const auto& mmdMat = m_mmdModel->GetMaterials()[i];
			auto& matRes = res.mMmdFragUniformOffset[i];
			auto mmdFSUB = reinterpret_cast<MMDFragmentShaderUB*>(ubPtr + matRes);

			mmdFSUB->m_alpha = mmdMat.m_alpha;
			mmdFSUB->m_diffuse = mmdMat.m_diffuse;
			mmdFSUB->m_ambient = mmdMat.m_ambient;
			mmdFSUB->m_specular = mmdMat.m_specular;
			mmdFSUB->m_specularPower = mmdMat.m_specularPower;
			// Tex
			if (!mmdMat.m_texture.empty())
			{
				if (!mMmdMaterials[i]->hasAlpha())
				{
					mmdFSUB->m_textureModes.x = 1;
				}
				else
				{
					mmdFSUB->m_textureModes.x = 2;
				}
				mmdFSUB->m_texMulFactor = mmdMat.m_textureMulFactor;
				mmdFSUB->m_texAddFactor = mmdMat.m_textureAddFactor;
			}
			else
			{
				mmdFSUB->m_textureModes.x = 0;
			}

			mmdFSUB->m_lightColor = appContext.m_lightColor;
			glm::vec3 lightDir = appContext.m_lightDir;
			auto viewMat = glm::mat3(appContext.m_viewMat);
			lightDir = viewMat * lightDir;
			mmdFSUB->m_lightDir = lightDir;
		}
	}
	device.unmapMemory(ubStBuf->m_memory);

	ubStBuf->CopyBuffer(modelRes.mUniformBuffer->getBuffer(), ubMemSize);
}

void Model::Draw(AppContext& appContext)
{
	auto inheritanceInfo = vk::CommandBufferInheritanceInfo()
		.setRenderPass(appContext.m_renderPass)
		.setSubpass(0);
	auto beginInfo = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue)
		.setPInheritanceInfo(&inheritanceInfo);

	auto& res = m_resource;
	auto& modelRes = res.m_modelResource;
	auto& cmdBuf = m_cmdBuffers[appContext.m_imageIndex];

	auto width = appContext.m_screenWidth;
	auto height = appContext.m_screenHeight;

	cmdBuf.begin(beginInfo);

	auto viewport = vk::Viewport()
		.setWidth(float(width))
		.setHeight(float(height))
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f);
	cmdBuf.setViewport(0, 1, &viewport);

	auto scissor = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height));
	cmdBuf.setScissor(0, 1, &scissor);

	vk::DeviceSize offsets[1] = { 0 };
	cmdBuf.bindVertexBuffers(0, 1, &modelRes.mVertexBuffer->getBuffer(), offsets);
	cmdBuf.bindIndexBuffer(mIndexBuffer->getBuffer(), 0, m_indexType);

	// MMD
	vk::Pipeline* currentMMDPipeline = nullptr;
	size_t subMeshCount = m_mmdModel->GetSubMeshCount();
	for (size_t i = 0; i < subMeshCount; i++)
	{
		const auto& subMesh = m_mmdModel->GetSubMeshes()[i];
		const auto& matID = subMesh.m_materialID;

		const auto& mmdMat = m_mmdModel->GetMaterials()[matID];
		auto& matRes = res.mMmdFragUniformOffset[matID];

		if (mmdMat.m_alpha == 0.0f)
		{
			continue;
		}

        cmdBuf.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                appContext.m_mmdPipelineLayout,
                0, 1, &appContext.mMmdDescriptor.getDescriptorSets()[i],
                0, nullptr);

		vk::Pipeline* mmdPipeline = nullptr;
		if (mmdMat.m_bothFace)
		{
			mmdPipeline = &appContext.m_mmdPipelines;
		}
		if (currentMMDPipeline != mmdPipeline)
		{
			cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, *mmdPipeline);
			currentMMDPipeline = mmdPipeline;
		}
		cmdBuf.drawIndexed(subMesh.m_vertexCount, 1, subMesh.m_beginIndex, 0, 1);
	}

	cmdBuf.end();
}

vk::CommandBuffer Model::GetCommandBuffer(uint32_t imageIndex) const
{
	return m_cmdBuffers[imageIndex];
}


class App
{
public:
	explicit App(GLFWwindow* window);
	bool Run();
	void Exit();
	void Resize(uint16_t w, uint16_t h);
	[[nodiscard]] uint16_t GetWidth() const { return m_width; }
	[[nodiscard]] uint16_t GetHeight() const { return m_height; }

	void MainLoop();
private:

private:
	GLFWwindow*			m_window = nullptr;
	uint16_t			m_width = 1280;
	uint16_t			m_height = 800;

	vk::Instance		m_vkInst;
	vk::SurfaceKHR		m_surface;
	vk::PhysicalDevice	m_gpu;
	vk::Device			m_device;

	AppContext	m_appContext;
	std::vector<Model>	m_models;

    double fpsTime;
    int fpsFrame{0};
    double saveTime;
    int frame{0};
    std::vector<vk::Semaphore> waitSemaphores;
    std::vector<vk::PipelineStageFlags> waitStageMasks;
};

App::App(GLFWwindow* window)
{
	m_window = window;
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	m_width = uint16_t(fbWidth);
	m_height = uint16_t(fbHeight);
//	m_newSize = 0;
}

bool App::Run()
{
    std::vector<Input> inputModels{{R"(E:\Material\model\Nilou\Nilou.pmx)", {R"(E:\Material\vmd\mmd\8.vmd)"}}};

    vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{};
    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtFeature{};
    yic::contextCreateInfo createInfo{};
    createInfo.addInstanceLayers("VK_LAYER_KHRONOS_validation")
            .addInstanceExtensions("VK_EXT_debug_utils")
            .addPhysicalDeviceExtensions("VK_KHR_swapchain")
            .addPhysicalDeviceExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelFeature)
            .addPhysicalDeviceExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rtFeature)
            .addPhysicalDeviceExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

    yic::vk_init::get(createInfo)->oneTimeInitialization();

    m_vkInst = yic::vk_init::get()->getInstance();
    m_surface = yic::vk_init::get()->surfaceKhr();
    m_gpu = yic::vk_init::get()->physicalDevice();
    m_device = yic::vk_init::get()->device();
    m_appContext.m_screenWidth = m_width;
    m_appContext.m_screenHeight = m_height;

	if (!m_appContext.Setup(yic::vk_init::get()))
	{
		std::cout << "Failed to setup.\n";
		return false;
	}

	for (const auto& input : inputModels)
	{
		// Load MMD model
		Model model;
		auto ext = saba::PathUtil::GetExt(input.m_modelPath);
        if (ext == "pmx")
		{
			auto pmxModel = std::make_unique<saba::PMXModel>();
			if (!pmxModel->Load(input.m_modelPath, m_appContext.m_mmdDir))
			{
				std::cout << "Failed to load pmx file.\n";
				return false;
			}
			model.m_mmdModel = std::move(pmxModel);
		}
		model.m_mmdModel->InitializeAnimation();

		// Load VMD animation
		auto vmdAnim = std::make_unique<saba::VMDAnimation>();
		if (!vmdAnim->Create(model.m_mmdModel))
		{
			std::cout << "Failed to create VMDAnimation.\n";
			return false;
		}
		for (const auto& vmdPath : input.m_vmdPaths)
		{
			saba::VMDFile vmdFile;
			if (!saba::ReadVMDFile(&vmdFile, vmdPath.c_str()))
			{
				std::cout << "Failed to read VMD file.\n";
				return false;
			}
			if (!vmdAnim->Add(vmdFile))
			{
				std::cout << "Failed to add VMDAnimation.\n";
				return false;
			}
		}
		vmdAnim->SyncPhysics(0.0f);

		model.m_vmdAnim = std::move(vmdAnim);

		model.Setup(m_appContext);

		m_models.emplace_back(std::move(model));
	}

    fpsTime = saba::GetTime();
    fpsFrame = 0;
    saveTime = saba::GetTime();
    frame = 0;

	return true;
}

void App::Exit()
{
	if (!m_device)
	{
		return;
	}

	m_device.waitIdle();
}

void App::MainLoop()
{
		frame++;


		vk::Result ret;
		auto& frameSyncData = m_appContext.m_frameSyncDatas[m_appContext.m_frameIndex];
		auto waitFence = frameSyncData.m_fence;
		auto renderCompleteSemaphore = frameSyncData.m_renderCompleteSemaphore;
		auto presentCompleteSemaphore = frameSyncData.m_presentCompleteSemaphore;

		m_device.waitForFences(1, &waitFence, true, UINT64_MAX);
		m_device.resetFences(1, &waitFence);

		do
		{
			ret = m_device.acquireNextImageKHR(
				m_appContext.m_swapchain,
				UINT64_MAX,
				presentCompleteSemaphore,
				vk::Fence(),
				&m_appContext.m_imageIndex
			);
			if (vk::Result::eErrorOutOfDateKHR == ret)
			{
			}
			else if (vk::Result::eSuccess != ret)
			{
				std::cout << "acquireNextImageKHR error.[" << uint32_t(ret) << "]\n";
				return;
			}
		} while (vk::Result::eSuccess != ret);

		//FPS
		{
			fpsFrame++;
			double time = saba::GetTime();
			double deltaTime = time - fpsTime;
			if (deltaTime > 1.0)
			{
				double fps = double(fpsFrame) / deltaTime;
				std::cout << fps << " fps\n";
				fpsFrame = 0;
				fpsTime = time;
			}
		}

		double time = saba::GetTime();
		double elapsed = time - saveTime;
		if (elapsed > 1.0 / 30.0)
		{
			elapsed = 1.0 / 30.0;
		}
		saveTime = time;
		m_appContext.m_elapsed = float(elapsed);
		m_appContext.m_animTime += float(elapsed);

		// Setup Camera
		int screenWidth = m_appContext.m_screenWidth;
		int screenHeight = m_appContext.m_screenHeight;

        m_appContext.m_viewMat = glm::lookAt(glm::vec3(0, 10, 50), glm::vec3(0, 10, 0), glm::vec3(0, 1, 0));
        m_appContext.m_projMat = glm::perspectiveFovRH(glm::radians(30.0f), float(screenWidth), float(screenHeight), 1.0f, 10000.0f);

		auto imgIndex = m_appContext.m_imageIndex;
		auto& res = m_appContext.m_swapchainImageResouces[imgIndex];
		auto& cmdBuf = res.m_cmdBuffer;

		// Update / Draw Models
		for (auto& model : m_models)
		{
			// Update Animation
			model.UpdateAnimation(m_appContext);
			// Update Vertices
			model.Update(m_appContext);
			// Draw
			model.Draw(m_appContext);
		}

		auto beginInfo = vk::CommandBufferBeginInfo();
		cmdBuf.begin(beginInfo);

		auto clearColor = vk::ClearColorValue(std::array<float, 4>({ 1.0f, 0.8f, 0.75f, 1.0f }));
		auto clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
		vk::ClearValue clearValues[] = {clearColor,clearDepth};

		auto renderPassBeginInfo = vk::RenderPassBeginInfo()
			.setRenderPass(m_appContext.m_renderPass)
			.setFramebuffer(res.m_framebuffer)
			.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(screenWidth, screenHeight)))
			.setClearValueCount(std::extent<decltype(clearValues)>::value)
			.setPClearValues(clearValues);
		cmdBuf.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eSecondaryCommandBuffers);

		for (auto& model : m_models)
		{
			auto modelCmdBuf = model.GetCommandBuffer(imgIndex);
			cmdBuf.executeCommands(1, &modelCmdBuf);
		}

		cmdBuf.endRenderPass();
		cmdBuf.end();

		waitSemaphores.push_back(presentCompleteSemaphore);
		waitStageMasks.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		for (const auto& stBuf : StagingBuffer::mStagingBuffers)
		{
			if (stBuf->m_waitSemaphore)
			{
				waitSemaphores.push_back(stBuf->m_waitSemaphore);
				waitStageMasks.push_back(vk::PipelineStageFlagBits::eTransfer);
				stBuf->m_waitSemaphore = nullptr;
			}
		}
		vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		auto submitInfo = vk::SubmitInfo()
			.setPWaitDstStageMask(waitStageMasks.data())
			.setWaitSemaphoreCount(uint32_t(waitSemaphores.size()))
			.setPWaitSemaphores(waitSemaphores.data())
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(&renderCompleteSemaphore)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmdBuf);
		ret = m_appContext.m_graphicsQueue.submit(1, &submitInfo, waitFence);
		waitSemaphores.clear();
		waitStageMasks.clear();

		auto presentInfo = vk::PresentInfoKHR()
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&renderCompleteSemaphore)
			.setSwapchainCount(1)
			.setPSwapchains(&m_appContext.m_swapchain)
			.setPImageIndices(&m_appContext.m_imageIndex);
		ret = m_appContext.m_graphicsQueue.presentKHR(&presentInfo);
		if (vk::Result::eSuccess != ret &&
			vk::Result::eErrorOutOfDateKHR != ret)
		{
			std::cout << "presetKHR error.[" << uint32_t(ret) << "]\n";
			return;
		}

		m_appContext.m_frameIndex++;
		m_appContext.m_frameIndex = m_appContext.m_frameIndex % m_appContext.m_imageCount;
//	}
}

bool SampleMain()
{
    yic::Window::get(1280, 800, "yicvot")->glfwCallback();

    auto app = std::make_unique<App>(yic::Window::get()->window());

    app->Run();
	while (!glfwWindowShouldClose(yic::Window::get()->window()))
	{
        app->MainLoop();
        glfwPollEvents();
	}
	app->Exit();
	app.reset();
	glfwTerminate();
	return true;
}

int main()
{
    SampleMain();

	return 0;
}
