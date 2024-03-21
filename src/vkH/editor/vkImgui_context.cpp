//
// Created by lenovo on 1/5/2024.
//

#include "vkImgui_context.h"

namespace ImRender {

    vkImgui_context::vkImgui_context(yic::vk_init *vkInit, yic::vkSwapchain &swapchain) : mSwapchain{swapchain} {
        mInstance = vkInit->getInstance();
        mPhysicalDevice = vkInit->physicalDevice();
        mDevice = vkInit->device();
        mGraphicsQueue = vkInit->graphicsQueue();
        mGraphicsIndex = vkInit->getGraphicsFamilyIndices();
        mExtent = mSwapchain.getExtent();

        taskAssignment();
    }

    vkImgui_context &vkImgui_context::taskAssignment() {
        taskerBuilder::toInit::addGeneralSequentialTask([this]{
            vk::CommandPoolCreateInfo createInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, mGraphicsIndex};
            vkCreate([&](){ mCommandPool = mDevice.createCommandPool(createInfo); }, " create command pool ");

            mCommandBuffers.resize(mSwapchain.getImageCount());
            vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, mSwapchain.getImageCount()};
            vkCreate([&](){ mCommandBuffers = mDevice.allocateCommandBuffers(allocateInfo); }, "allocate commandBuffers");
        });
        taskerBuilder::toInit::addGeneralSequentialTask([this]{
            this->createRenderPass()
                .createFrameBuffer()
                .createGraphicsPipeline();
            mDescriptor.increaseMaxSets()
                    .createDescriptorPool();
        });

        taskerBuilder::toTransfer::addTexture(mImagePng2, texPath "5.png");

        taskerBuilder::toGraphics::transformTexLayout([this]{this->updateDescriptorSet();});

        taskerBuilder::toPreRender::addGeneralSequentialTask([&]{
            imguiContext context{Window::get()->window(), mInstance, mPhysicalDevice, mDevice, mGraphicsQueue,
                                 mRenderPass, mDescriptor.getDescriptorPool(), mSwapchain.getImageCount()};
            mvkImgui = std::make_unique<vkImgui>(context);
            mvkImgui->setShowDemo(false);
        });

        return *this;
    }


    vkImgui_context &vkImgui_context::createFrameBuffer() {
        for(auto& framebuffer : mFrameBuffers){
            mDevice.destroy(framebuffer);
        }

        std::array<vk::ImageView, 1> attachments{};
        vk::FramebufferCreateInfo createInfo{{}, mRenderPass, attachments,
                                             mExtent.width, mExtent.height, 1};

        mFrameBuffers.resize(mSwapchain.getImageCount());
        for(uint32_t i = 0; i < mSwapchain.getImageCount(); i++){
            attachments[0] = mSwapchain.getImageView(i);
            vkCreate([&](){ mFrameBuffers[i] = mDevice.createFramebuffer(createInfo);}, "create framebuffer" + std::to_string(i), 0);
        }

        return *this;
    }

    vkImgui_context &vkImgui_context::createRenderPass() {
        assert(mSwapchain.getSwapchain() != VK_NULL_HANDLE && "you must create swapchain before render pass, please call the create swapchain first. :)");
        std::vector<vk::AttachmentDescription> attachmentDescription{{{},
                                                                             mSwapchain.getSurfaceFormat(), vk::SampleCountFlagBits::e1,
                                                                             vk::AttachmentLoadOp::eLoad,
                                                                             vk::AttachmentStoreOp::eStore,
                                                                             vk::AttachmentLoadOp::eDontCare,
                                                                             vk::AttachmentStoreOp::eDontCare,
                                                                             vk::ImageLayout::eColorAttachmentOptimal,
                                                                             vk::ImageLayout::ePresentSrcKHR},};
        std::vector<vk::AttachmentReference> attachmentReference{{0, vk::ImageLayout::eColorAttachmentOptimal}};

        vk::SubpassDescription subpass{{}, vk::PipelineBindPoint::eGraphics, {}, attachmentReference, {}, {}};

        vk::RenderPassCreateInfo createInfo{{}, attachmentDescription, subpass, {}};
        vkCreate([&](){ mRenderPass = mDevice.createRenderPass(createInfo); }, "create renderPass");
        return *this;
    }

    vkImgui_context &vkImgui_context::createGraphicsPipeline() {
        mDescriptor.addDescriptorSetLayout({
            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
        });

        yic::graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mDescriptor.getPipelineLayout(), mRenderPass};
        pipelineCombined.depthStencilState.setDepthTestEnable(vk::False)
                                        .setDepthWriteEnable(vk::False);
        pipelineCombined.addShader(ke_q::loadFile("vImgui.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("fImgui.spv"), vk::ShaderStageFlagBits::eFragment);
        mGraphicsPipeline = pipelineCombined.createGraphicsPipeline();

        return *this;
    }



    vkImgui_context &vkImgui_context::updateDescriptorSet() {

        mDescriptor.createDescriptorSets();

        auto cmd = createTempCmdBuf();
        mImagePng2->configureImageForRender(cmd);
        submitTempCmdBuf(cmd);

        mDescriptor.addDescriptorSet({
                                             vk::DescriptorImageInfo{mImagePng2->getSampler(),
                                                                     mImagePng2->getImageView(),
                                                                     vk::ImageLayout::eShaderReadOnlyOptimal}
                                     }).update();

        return *this;
    }

    vkImgui_context &vkImgui_context::renderImgui() {
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        std::vector<vk::ClearValue> clearValues{vk::ClearColorValue{0.2f, 0.3f, 0.2f, 1.f}};
        uint32_t imageIndex = mSwapchain.getActiveImageIndex();
        auto& cmd = mCommandBuffers[imageIndex];
        cmd.begin(beginInfo);
        {
            vk::RenderPassBeginInfo renderPassBeginInfo{mRenderPass, mFrameBuffers[imageIndex],
                                                        {{0, 0}, mExtent}, {}};

            {
                mCommandBuffers[imageIndex].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

                mvkImgui->beginRenderImgui();
                mvkImgui->fixedFrame(mExtent);
                if (mImagePng2 != nullptr)
                    mvkImgui->RenderViewWindow(mDescriptor.getDescriptorSets().front(), ImVec2{(float)mImagePng2->getImageSize().width * 0.35f, (float)mImagePng2->getImageSize().height * 0.35f});
                else
                    mvkImgui->RenderViewWindow();
                mvkImgui->vkRenderWindow();
                mvkImgui->endRenderImgui(cmd);
            }
            cmd.endRenderPass();
        }
        cmd.end();
        return *this;
    }

    vkImgui_context &vkImgui_context::recreate() {
        int w, h;
        glfwGetFramebufferSize(Window::get()->window(), &w, &h);

        if (w != (int)mExtent.width || h != (int)mExtent.height){
            if (w == 0 || h == 0){

            } else{
                mDevice.waitIdle();
                mGraphicsQueue.waitIdle();

                mExtent = mSwapchain.getExtent();
                createFrameBuffer();
            }
        }
        return *this;
    }

    vk::CommandBuffer vkImgui_context::createTempCmdBuf() {
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, 1};
        vk::CommandBuffer cmd = mDevice.allocateCommandBuffers(allocateInfo).front();

        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        cmd.begin(beginInfo);

        return cmd;
    }

    void vkImgui_context::submitTempCmdBuf(vk::CommandBuffer& cmd) {
        cmd.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmd);
        mGraphicsQueue.submit(submitInfo);
        mGraphicsQueue.waitIdle();
        mDevice.free(mCommandPool, cmd);
    }



} // lmRender