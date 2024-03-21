//
// Created by lenovo on 2/3/2024.
//

#include "vkSkybox.h"

namespace yic {

    vkSkybox::vkSkybox(yic::vk_init *vkInit, yic::vkSwapchain &swapchain) : mSwapchain{swapchain} {
        mDevice = vkInit->device();
        mGraphicsQueue = vkInit->graphicsQueue();
        mExtent = swapchain.getExtent();
        mGraphicsIndex = vkInit->getGraphicsFamilyIndices();

        taskAssignment();
    }

    vkSkybox &vkSkybox::taskAssignment() {
        mCameraVecBuf = std::make_unique<genericBufferManager>(sizeof(cameraVec), vk::BufferUsageFlagBits::eUniformBuffer);

        taskerBuilder::toInit::addGeneralSequentialTask([this]{this->prepareSkyboxEveryThing();});

        taskerBuilder::toTransfer::addTexture(mSkybox, std::vector<std::string>{
                texPath "skybox/nz.png", texPath "skybox/pz.png",
                texPath "skybox/ny.png", texPath "skybox/py.png",
                texPath "skybox/nx.png", texPath "skybox/px.png",
        });

        taskerBuilder::toGraphics::transformTexLayout([this]{ this->updateSkyDescriptor(); });

        return *this;
    }

    vkSkybox &vkSkybox::renderSkybox() {
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        std::vector<vk::ClearValue> clearValues{vk::ClearColorValue{ 1.0f, 0.8f, 0.75f, 1.0f},{}};

        uint32_t imageIndex = mSwapchain.getActiveImageIndex();
        mCommandBuffers[imageIndex].begin(beginInfo);
        {
            vk::RenderPassBeginInfo renderPassBeginInfo{mSkyboxRenderPass[eSkybox], mSkyFrameBuffers[eSkybox][imageIndex],
                                                        {{0, 0}, mExtent}, clearValues};

            auto& cmd = mCommandBuffers[imageIndex];
            {
                cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

                vk::Viewport viewport{0.f, 0.f, static_cast<float>(mExtent.width), static_cast<float>(mExtent.height), 0.f, 1.f};
                cmd.setViewport(0, viewport);

                vk::Rect2D scissor{{0, 0}, {mExtent.width, mExtent.height}};
                cmd.setScissor(0, scissor);

                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mSkyboxPipeline[eSkybox]);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mSkyboxPipeLayout[eSkybox], 0, mSkyboxDescriptor[eSkybox].getDescriptorSets(),
                                       nullptr);
                cmd.draw(36, 1, 0, 0);
            }
            mCommandBuffers[imageIndex].endRenderPass();
        }
        mCommandBuffers[imageIndex].end();

        return *this;
    }

    vkSkybox &vkSkybox::prepareSkyboxEveryThing() {
        vk::CommandPoolCreateInfo createInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, mGraphicsIndex};
        vkCreate([&](){ mCommandPool = mDevice.createCommandPool(createInfo); }, " create command pool ");

        mCommandBuffers.resize(mSwapchain.getImageCount());
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, mSwapchain.getImageCount()};
        vkCreate([&](){ mCommandBuffers = mDevice.allocateCommandBuffers(allocateInfo); }, "allocate commandBuffers");

        createSkyRenderPass();
        createSkyFrameBuffer();
        createSkyboxPipeline();

        return *this;
    }

    vkSkybox &vkSkybox::createSkyRenderPass() {
        std::vector<vk::AttachmentDescription> attachmentDescription{{{},
                                                                      mSwapchain.getSurfaceFormat(), vk::SampleCountFlagBits::e1,
                                                                      vk::AttachmentLoadOp::eClear,
                                                                      vk::AttachmentStoreOp::eStore,
                                                                      vk::AttachmentLoadOp::eDontCare,
                                                                      vk::AttachmentStoreOp::eDontCare,
                                                                      vk::ImageLayout::eUndefined,
                                                                      vk::ImageLayout::eColorAttachmentOptimal},};
        std::vector<vk::AttachmentReference> attachmentReference{{0, vk::ImageLayout::eColorAttachmentOptimal}};

        vk::SubpassDescription subpass{{}, vk::PipelineBindPoint::eGraphics, {}, attachmentReference, {}, {}};

        vk::RenderPassCreateInfo createInfo{{}, attachmentDescription, subpass, {}};
        vkCreate([&](){ mSkyboxRenderPass[eSkybox] = mDevice.createRenderPass(createInfo); }, "create renderPass");

        return *this;
    }

    vkSkybox &vkSkybox::createSkyFrameBuffer() {
        for(auto& framebuffer : mSkyFrameBuffers[eSkybox]){
            mDevice.destroy(framebuffer);
        }

        std::array<vk::ImageView, 1> attachments{};
        vk::FramebufferCreateInfo createInfo{{}, mSkyboxRenderPass[eSkybox], attachments,
                                             mExtent.width, mExtent.height, 1};

        mSkyFrameBuffers[eSkybox].resize(mSwapchain.getImageCount());
        for(uint32_t i = 0; i < mSwapchain.getImageCount(); i++){
            attachments[0] = mSwapchain.getImageView(i);
            vkCreate([&](){ mSkyFrameBuffers[eSkybox][i] = mDevice.createFramebuffer(createInfo);}, "create framebuffer" + std::to_string(i), 0);
        }

        return *this;
    }

    vkSkybox &vkSkybox::createSkyboxPipeline() {
        mSkyboxDescriptor[skyPipeTypes::eSkybox].addDescriptorSetLayout({
                vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
            }).addDescriptorSetLayout({
                vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
            });

        mSkyboxPipeLayout[skyPipeTypes::eSkybox] = mSkyboxDescriptor[skyPipeTypes::eSkybox].getPipelineLayout();
        yic::graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mSkyboxPipeLayout[skyPipeTypes::eSkybox], mSkyboxRenderPass[skyPipeTypes::eSkybox]};
        pipelineCombined.depthStencilState.setDepthTestEnable(vk::False)
                .setDepthWriteEnable(vk::False);
        pipelineCombined.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
        pipelineCombined.addShader(ke_q::loadFile("vSky.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("fSky.spv"), vk::ShaderStageFlagBits::eFragment);
        mSkyboxPipeline[skyPipeTypes::eSkybox] = pipelineCombined.createGraphicsPipeline();

        return *this;
    }



    vkSkybox& vkSkybox::updateSkyDescriptor() {
        auto cmd = createTempCmdBuf();
        mSkybox->configureImageForRender(cmd);
        submitTempCmdBuf(cmd);

        mSkyboxDescriptor[eSkybox]
                .createDescriptorPool()
                .createDescriptorSets()
                .addDescriptorSet({
                                          vk::DescriptorBufferInfo{mCameraVecBuf->getBuffer(), 0, sizeof(cameraVec)}
                                  })
                .addDescriptorSet({
                                          vk::DescriptorImageInfo{mSkybox->getSampler(), mSkybox->getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal}
                                  })
                .update();

        return *this;
    }

    vkSkybox &vkSkybox::updateCameraVec() {
        glm::mat4 view = vkCamera::get()->getViewMatrix();
        view[3] = glm::vec4(0.f, 0.f, 0.f, view[3].w);
        mCameraVec.vpMatrix = vkCamera::get()->getProjMatrix(mSwapchain.getExtent()) * view;

        mCameraVecBuf->updateBuffer(mCameraVec);

        return *this;
    }

    vkSkybox &vkSkybox::recreate() {
        int w, h;
        glfwGetFramebufferSize(Window::get()->window(), &w, &h);

        if (w != (int)mExtent.width || h != (int)mExtent.height){
            if (w == 0 || h == 0){

            } else{
                mDevice.waitIdle();
                mGraphicsQueue.waitIdle();

                mExtent = mSwapchain.getExtent();
                createSkyFrameBuffer();
            }
        }
        return *this;
    }

    vk::CommandBuffer vkSkybox::createTempCmdBuf() {
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, 1};
        vk::CommandBuffer cmd = mDevice.allocateCommandBuffers(allocateInfo).front();

        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        cmd.begin(beginInfo);

        return cmd;
    }

    void vkSkybox::submitTempCmdBuf(vk::CommandBuffer& cmd) {
        cmd.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmd);
        mGraphicsQueue.submit(submitInfo);
        mGraphicsQueue.waitIdle();
        mDevice.free(mCommandPool, cmd);
    }
















} // yic

















//        auto& id = mSkyboxDescriptor[eSkybox].getId();

//        tasker::toResource::addTexture(mSkybox, paths, id);
//        tasker::toResource::specialTask::markGroupEnd(id);
//        tasker::toResource::specialTask::promise::pre_task(id);
//
//        tasker::toResource::specialTask::promise::post_task(id);
//        tasker::toResource::eventListeners::subscribe(id, [this]{this->updateSkyDescriptor();});