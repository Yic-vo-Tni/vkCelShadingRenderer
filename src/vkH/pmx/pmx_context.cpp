//
// Created by lenovo on 1/13/2024.
//

#include "pmx_context.h"


namespace vkPmx {
    
    pmx_context::pmx_context(const std::vector<Input>& inputModels, yic::vk_init *vkInit, uint32_t imageCount, const vk::RenderPass &renderPass,
                             const vk::CommandPool &commandPool) : mvkInit{vkInit}, mImageCount(imageCount), mRenderPass(renderPass), mCommandPool(commandPool){
        mDevice = vkInit->device();
        mPhysicalDevice = vkInit->physicalDevice();

        mResDir = saba::PathUtil::GetExecutablePath();
        mResDir = saba::PathUtil::GetDirectoryName(mResDir);
        mResDir = saba::PathUtil::Combine(mResDir, "resource");
        mShaderDir = saba::PathUtil::Combine(mResDir, "shader");
        mMmdDir = saba::PathUtil::Combine(mResDir, "mmd");

        loadPmxInputModels(inputModels);

        mSaveTime = saba::GetTime();

        mMaterialCount = mMmdModel->GetMaterialCount();
        mMmdMaterials.resize(mMaterialCount);

        prepareEveryThing();
    }

    pmx_context &pmx_context::prepareEveryThing() {
        createPipeline();
        createVertexBuffer();
        createCommandBuffer();
        createDescriptorSet();

        return *this;
    }

    pmx_context &pmx_context::updateEveryFrame(const glm::mat4& view, const glm::mat4& proj){
        double time = saba::GetTime();
        double elapsed = time - mSaveTime;
        if (elapsed > 1.f / 30.f){
            elapsed = 1.f / 30.f;
        }
        mSaveTime = time;
        mElapsed = float (elapsed);
        mAnimTime += float (elapsed);

        mView = view;
        mProj = proj;

        return *this;
    }

    pmx_context &pmx_context::createPipeline() {
        mMmdDescriptor.addDescriptorSetLayout({
           vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
           vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
           vk::DescriptorSetLayoutBinding{2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
        });

        mPipelineLayout = mMmdDescriptor.getPipelineLayout();
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mPipelineLayout, mRenderPass};
        pipelineCombined.rasterizationState.setFrontFace(vk::FrontFace::eCounterClockwise)
                .setDepthClampEnable(false).setRasterizerDiscardEnable(false)
                .setDepthBiasEnable(false);
        pipelineCombined.depthStencilState.setBack(vk::StencilOpState().setFailOp(vk::StencilOp::eKeep).setPassOp(vk::StencilOp::eKeep).setCompareOp(vk::CompareOp::eAlways));
        pipelineCombined.depthStencilState.front = pipelineCombined.depthStencilState.back;
        pipelineCombined.addShader(ke_q::loadFile("v_mmd.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_mmd.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription({0, sizeof(Vertex), vk::VertexInputRate::eVertex});
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, m_position)},
                                                          {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, m_normal)},
                                                          {2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, m_uv)},
                                                  });
        vk::PipelineColorBlendAttachmentState state{yic::graphicsPipelineState::makePipelineColorBlendAttachments({}, true,
                                                                                                                  vk::BlendFactor::eSrcAlpha,vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
                                                                                                                  vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha,vk::BlendOp::eAdd)};
        pipelineCombined.updateState();
        pipelineCombined.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
        mPipeline = pipelineCombined.createGraphicsPipeline();

        return *this;
    }

    pmx_context &pmx_context::createVertexBuffer() {
        /// vertex buffer
        auto vertBufMemSize = uint32_t (sizeof (Vertex)) * mMmdModel->GetVertexCount();
        mModelResource.mVertexBuffer = std::make_unique<yic::genericBufferManager>(vertBufMemSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
        
        /// index buffer
        auto indexMemSize = uint32_t (mMmdModel->GetIndexCount() * mMmdModel->GetIndexElementSize());
        mModelResource.mIndexBuffer = std::make_unique<yic::genericBufferManager>(indexMemSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
        // 
        StagingBuffer* indicesStagingBuffer;
        StagingBuffer::getStagingBuffer(indexMemSize, &indicesStagingBuffer);
        
        void* mapMem;
        if ( mDevice.mapMemory(indicesStagingBuffer->m_memory, 0, indexMemSize, vk::MemoryMapFlagBits(0), &mapMem) != vk::Result::eSuccess ){
            vkError("failed to map memory");
        }
        memcpy(mapMem, mMmdModel->GetIndices(), indexMemSize);
        mDevice.unmapMemory(indicesStagingBuffer->m_memory);

        indicesStagingBuffer->CopyBuffer(mModelResource.mIndexBuffer->getBuffer(), indexMemSize);
        indicesStagingBuffer->Wait();
        
        /// find index element size
        switch (mMmdModel->GetIndexElementSize()) {
            case 2:
                mModelResource.mIndexType = vk::IndexType::eUint16;
                break;
            case 4:
                mModelResource.mIndexType = vk::IndexType::eUint32;
                break;
            default:
                vkError("unknown index size {0}", mMmdModel->GetIndexElementSize());
                break;
        }

        return *this;
    }

    pmx_context &pmx_context::createMaterialsTex() {
        for(size_t i = 0; i < mMaterialCount; i++){
            tasker::toResource::addTexture(mMmdMaterials[i], mMmdModel->GetMaterials()[i].m_texture, mMmdDescriptor.getId());
        }
        tasker::toResource::specialTask::markGroupEnd(mMmdDescriptor.getId());

        return *this;
    }

    pmx_context &pmx_context::createDescriptorSet() {
        uint32_t uniformAlign = uint32_t (mPhysicalDevice.getProperties().limits.minUniformBufferOffsetAlignment);

        uint32_t uniformBufOffset{0};
        mModelResource.mMmdVertUniformBufOffset = uniformBufOffset;
        uniformBufOffset += sizeof (MMDVertxShaderUB);
        uniformBufOffset = (uniformBufOffset + uniformAlign) - ((uniformBufOffset + uniformAlign) % uniformAlign);

        mModelResource.mMmdFragUniformBufOffset.resize(mMaterialCount);
        for(size_t i = 0; i < mMaterialCount; i++){
            mModelResource.mMmdFragUniformBufOffset[i] = uniformBufOffset;
            uniformBufOffset += sizeof (MMDFragmentShaderUB);
            uniformBufOffset = (uniformBufOffset + uniformAlign) - ((uniformBufOffset + uniformAlign) % uniformAlign);
        }

        mModelResource.mUniformBuffer = std::make_unique<yic::genericBufferManager>(uniformBufOffset, vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst);

        // descriptor
        mMmdDescriptor.increaseMaxSets(mMaterialCount - 1)
                .createDescriptorPool();

        createMaterialsTex();

        tasker::toResource::eventListeners::subscribe(mMmdDescriptor.getId(),[&]() {
            auto cmd = createTempCmdBuf();
            for (size_t i = 0; i < mMaterialCount; i++) {
                mMmdMaterials[i]->configureImageForRender(cmd);
            }
            submitTempCmdBuf(cmd);


            for (size_t i = 0; i < mMaterialCount; i++) {
                        mMmdDescriptor.pushBackDesSets();
                        mMmdDescriptor.updateDescriptorSet({
                                                                   vk::DescriptorBufferInfo{
                                                                           mModelResource.mUniformBuffer->getBuffer(),
                                                                           mModelResource.mMmdVertUniformBufOffset,
                                                                           sizeof(MMDVertxShaderUB)},
                                                                   vk::DescriptorBufferInfo{
                                                                           mModelResource.mUniformBuffer->getBuffer(),
                                                                           mModelResource.mMmdFragUniformBufOffset[i],
                                                                           sizeof(MMDFragmentShaderUB)},
                                                                   vk::DescriptorImageInfo{
                                                                           mMmdMaterials[i]->getSampler(),
                                                                           mMmdMaterials[i]->getImageView(),
                                                                           vk::ImageLayout::eShaderReadOnlyOptimal},
                                                           });
                    }
                });

        return *this;
    }

    pmx_context &pmx_context::createCommandBuffer() {
        mCmdBuffers.resize(mImageCount);

        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::eSecondary, mImageCount};
        mCmdBuffers = mDevice.allocateCommandBuffers(allocateInfo);

        return *this;
    }

    pmx_context &pmx_context::updateAnimation() {
        mMmdModel->BeginAnimation();
        mMmdModel->UpdateAllAnimation(mVmdAnimation.get(), mAnimTime * 30.f, mElapsed);
        mMmdModel->EndAnimation();

        return *this;
    }

    pmx_context &pmx_context::update() {
        size_t vertexCount = mMmdModel->GetVertexCount();
        mMmdModel->Update();
        auto position{mMmdModel->GetUpdatePositions()};
        auto normal(mMmdModel->GetUpdateNormals());
        auto uv{mMmdModel->GetUpdateUVs()};

        /// update vertices buffer
        auto memSize{vk::DeviceSize{sizeof (Vertex) * vertexCount}};
        StagingBuffer* vertBuf;
        StagingBuffer::getStagingBuffer(memSize, &vertBuf);
        void* vertStagingBufMem;
        if(mDevice.mapMemory(vertBuf->m_memory, 0, memSize, vk::MemoryMapFlags(), &vertStagingBufMem) != vk::Result::eSuccess){
            vkError("failed to map memory vert stag");
        }

        auto vertex = static_cast<Vertex*>(vertStagingBufMem);
        for(size_t i = 0; i < vertexCount; i++){
            vertex[i] = Vertex(position[i], normal[i], uv[i]);
        }
        mDevice.unmapMemory(vertBuf->m_memory);
        vertBuf->CopyBuffer(mModelResource.mVertexBuffer->getBuffer(), memSize);

        /// update uniform buffer
        auto uniformSize = mModelResource.mUniformBuffer->getDeviceSize();
        StagingBuffer* uniformBuf;
        StagingBuffer::getStagingBuffer(uniformSize, &uniformBuf);
        uint8_t* uniformBufPtr;
        if (mDevice.mapMemory(uniformBuf->m_memory, 0, uniformSize, vk::MemoryMapFlags(), (void**)&uniformBufPtr) != vk::Result::eSuccess){
            vkError("failed to map mem uniform stag");
        }
        // model unif
        auto mmdVertUnifBuf = reinterpret_cast<MMDVertxShaderUB*>(uniformBufPtr + mModelResource.mMmdVertUniformBufOffset);
        mmdVertUnifBuf->m_wv = mView;
        mmdVertUnifBuf->m_wvp = mProj * mView;
        // material unif
        for(size_t i = 0; i < mMaterialCount; i++){
            const auto& mmdMaterial = mMmdModel->GetMaterials()[i];
            auto mmdFragUnifBuf = reinterpret_cast<MMDFragmentShaderUB*>(uniformBufPtr + mModelResource.mMmdFragUniformBufOffset[i]);

            mmdFragUnifBuf->m_alpha = mmdMaterial.m_alpha;
            mmdFragUnifBuf->m_diffuse = mmdMaterial.m_diffuse;
            mmdFragUnifBuf->m_ambient = mmdMaterial.m_ambient;
            mmdFragUnifBuf->m_specular = mmdMaterial.m_specular;
            mmdFragUnifBuf->m_specularPower = mmdMaterial.m_specularPower;

            if (!mmdMaterial.m_texture.empty()){
                if (!mMmdMaterials[i]->hasAlpha()){
                    mmdFragUnifBuf->m_textureModes.x = 1;
                } else {
                    mmdFragUnifBuf->m_textureModes.x = 2;
                }
                mmdFragUnifBuf->m_texMulFactor = mmdMaterial.m_textureMulFactor;
                mmdFragUnifBuf->m_texAddFactor = mmdMaterial.m_textureAddFactor;
            } else {
                mmdFragUnifBuf->m_textureModes.x = 0;
            }

            mmdFragUnifBuf->m_lightColor = mLightColor;
            glm::vec3 lightDir = mLightDir;
            auto viewMaterial = glm::mat3 (mView);
            lightDir = viewMaterial * lightDir;
            mmdFragUnifBuf->m_lightDir = lightDir;
        }
        mDevice.unmapMemory(uniformBuf->m_memory);
        uniformBuf->CopyBuffer(mModelResource.mUniformBuffer->getBuffer(), uniformSize);

        return *this;
    }

    pmx_context &pmx_context::draw(const vk::CommandBuffer& cmd) {
        vk::DeviceSize offsets[] = {0};
        cmd.bindVertexBuffers(0, 1, &mModelResource.mVertexBuffer->getBuffer(), offsets);
        cmd.bindIndexBuffer(mModelResource.mIndexBuffer->getBuffer(), 0, mModelResource.mIndexType);

        vk::Pipeline *currentMmdPipe = nullptr;
        size_t subMeshCount = mMmdModel->GetSubMeshCount();
        for (size_t i = 0; i < subMeshCount; i++) {
            const auto &subMesh = mMmdModel->GetSubMeshes()[i];
            const auto &materialId = subMesh.m_materialID;

            const auto &mmdMaterial = mMmdModel->GetMaterials()[materialId];

            if (mmdMaterial.m_alpha == 0.f) continue;

            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout,
                                   0, 1, &mMmdDescriptor.getDescriptorSets()[i], 0, nullptr);

            vk::Pipeline *mmdPipe = nullptr;
            if (mmdMaterial.m_bothFace) mmdPipe = &mPipeline;
            if (currentMmdPipe != mmdPipe) {
                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *mmdPipe);
                currentMmdPipe = mmdPipe;
            }
            cmd.drawIndexed(subMesh.m_vertexCount, 1, subMesh.m_beginIndex, 0, 1);

        }
        return *this;
    }


/////////

    pmx_context &pmx_context::loadPmxInputModels(const std::vector<Input> &inputModels) {
        for(const auto& input : inputModels){
            auto ext = saba::PathUtil::GetExt(input.m_modelPath);
            if (ext == "pmx"){
                auto pmxModel = std::make_unique<saba::PMXModel>();
                if (!pmxModel->Load(input.m_modelPath, mMmdDir)){
                    vkError("failed to load pmx file");
                }
                mMmdModel = std::move(pmxModel);
            }
            mMmdModel->InitializeAnimation();

            auto vmdAnim = std::make_unique<saba::VMDAnimation>();
            if (!vmdAnim->Create(mMmdModel)){
                vkError("failed to create VMD anim");
            }
            for(const auto& vmdPath : input.m_vmdPaths){
                saba::VMDFile vmdFile;
                if (!saba::ReadVMDFile(&vmdFile, vmdPath.c_str())){
                    vkError("failed to read vmd file");
                }
                if (!vmdAnim->Add(vmdFile)){
                    vkError("failed to add vmd anim");
                }
            }
            vmdAnim->SyncPhysics(0.f);
            mVmdAnimation = std::move(vmdAnim);
        }

        return *this;
    }

    vk::CommandBuffer pmx_context::createTempCmdBuf() {
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, 1};
        vk::CommandBuffer cmd = mDevice.allocateCommandBuffers(allocateInfo).front();

        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        cmd.begin(beginInfo);

        return cmd;
    }
//15195546346
    void pmx_context::submitTempCmdBuf(vk::CommandBuffer& cmd) {
        cmd.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmd);
        mvkInit->graphicsQueue().submit(submitInfo);
        mvkInit->graphicsQueue().waitIdle();
        mDevice.free(mCommandPool, cmd);
    }





































} // vkPmx