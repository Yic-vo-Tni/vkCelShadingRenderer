//
// Created by lenovo on 12/19/2023.
//

#include "vk_base.h"

namespace yic {

    vk_base &vk_base::setup(yic::vk_init *vkInit) {
        mDevice = vkInit->device();
        mPhysicalDevice = vkInit->physicalDevice();
        mGraphicsIndex = vkInit->getGraphicsFamilyIndices();
        mGraphicsQueue = vkInit->graphicsQueue();

        vkAllocator::init(vkInit);
        mDesc.init(vkInit->device());

        vk::CommandPoolCreateInfo createInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, mGraphicsIndex};
        vkCreate([&](){ mCommandPool = mDevice.createCommandPool(createInfo); }, " create command pool ");

        return *this;
    }

    vk_base &vk_base::createSwapchain(const vk::SurfaceKHR &surface, Window* window,
                                      vk::Format colorFormat, vk::Format depthFormat) {
        mExtent = vk::Extent2D{(uint32_t )window->width(), (uint32_t )window->height()};
        mColorFormat = colorFormat;
        mDepthFormat = depthFormat; // format to be used

        vk_fn::getDepthFormat(mPhysicalDevice, mDepthFormat);

        mSwapchain.init(mDevice, mPhysicalDevice, mGraphicsQueue,
                        mGraphicsIndex, surface)
                  .upDate((uint32_t )window->width(), (uint32_t )window->height());

        mColorFormat = mSwapchain.getSurfaceFormat(); // the final format used

        mWaitFences.resize(mSwapchain.getImageCount());
        for(size_t i = 0; auto& fence : mWaitFences){
            vk::FenceCreateInfo createInfo{vk::FenceCreateFlagBits::eSignaled};
            vkCreate([&](){fence = mDevice.createFence(createInfo); }, "create fence" + std::to_string(i), 0);
            i++;
        }
        mCommandBuffers.resize(mSwapchain.getImageCount());
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, mSwapchain.getImageCount()};
        vkCreate([&](){ mCommandBuffers = mDevice.allocateCommandBuffers(allocateInfo); }, "allocate commandBuffers");

        return *this;
    }



    vk_base &vk_base::createDepthBuffer() {
        if (mDepthView)
            mDevice.destroy(mDepthView);
        if (mDepthImage)
            mDevice.destroy(mDepthImage);
        if (mDepthMemory)
            mDevice.free(mDepthMemory);

        vk::ImageCreateInfo createInfo{{},
                                       vk::ImageType::e2D, mDepthFormat, vk::Extent3D{mExtent, 1},
                                       1, 1, vk::SampleCountFlagBits::e1, {},
                                       vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled};
        vkCreate([&](){ mDepthImage = mDevice.createImage(createInfo); }, "create depth image");

        ///
        vk::MemoryRequirements memReqs{mDevice.getImageMemoryRequirements(mDepthImage)};
        vk::MemoryAllocateInfo allocateInfo{memReqs.size, vk_fn::getMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
        vkCreate([&](){ mDepthMemory = mDevice.allocateMemory(allocateInfo); }, "allocate depth memory");
        //
        mDevice.bindImageMemory(mDepthImage, mDepthMemory, 0);


        auto cmd = createTempCmdBuf();
        vk::ImageSubresourceRange subRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, {}, 1, {}, 1};
        vk::ImageMemoryBarrier memBarrier{{}, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                          vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                          {}, {}, mDepthImage, subRange};
        const vk::PipelineStageFlags srcStageFlag { vk::PipelineStageFlagBits::eTopOfPipe };
        const vk::PipelineStageFlags dstStageFlag { vk::PipelineStageFlagBits::eEarlyFragmentTests };

        cmd.pipelineBarrier(srcStageFlag, dstStageFlag, {}, {}, {}, memBarrier);
        //
        submitTempCmdBuf(cmd);

        mDepthView = vk_fn::createImageView(mDevice, mDepthImage, mDepthFormat, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);



        ////



        vk::ImageCreateInfo imageCreateInfo{
                {},
                vk::ImageType::e2D,
                mDepthFormat,  // 选择合适的深度格式
                {mExtent.width, mExtent.height, 1},
                1,  // Mip levels
                1,  // Array layers
                vk::SampleCountFlagBits::e1,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                vk::SharingMode::eExclusive,
                {},
                vk::ImageLayout::eUndefined  // 初始布局
        };
        newDepthImage = mDevice.createImage(imageCreateInfo);

        vk::MemoryRequirements memReqsR = mDevice.getImageMemoryRequirements(newDepthImage);
        vk::MemoryAllocateInfo allocateInfoR{
                memReqsR.size,
                vk_fn::getMemoryType(mPhysicalDevice, memReqsR.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
        };
        vk::DeviceMemory newDepthMemory = mDevice.allocateMemory(allocateInfoR);
        mDevice.bindImageMemory(newDepthImage, newDepthMemory, 0);

        updateRenderDepthImage();

        mRenderDepthView = vk_fn::createImageView(mDevice, newDepthImage, mDepthFormat, vk::ImageAspectFlagBits::eDepth);

        return *this;
    }

    vk_base &vk_base::updateRenderDepthImage() {
        auto cmdR = createTempCmdBuf();
        transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, cmdR, newDepthImage);
        transitionImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal, cmdR, mDepthImage);

        vk::ImageCopy copyRegion{
                {vk::ImageAspectFlagBits::eDepth, 0, 0, 1},
                {},
                {vk::ImageAspectFlagBits::eDepth, 0, 0, 1},
                {},
                {mExtent.width, mExtent.height, 1}
        };
        cmdR.copyImage(mDepthImage, vk::ImageLayout::eTransferSrcOptimal, newDepthImage, vk::ImageLayout::eTransferDstOptimal, copyRegion);

// Transition the new image to the shader readable layout
        transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, cmdR, newDepthImage);
        transitionImageLayout(vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal, cmdR, mDepthImage);
        submitTempCmdBuf(cmdR);

        return *this;
    }


    vk_base &vk_base::createOffscreenRenderPass() {
        std::vector<vk::AttachmentDescription> attachmentDes{
                { {},
                  mColorFormat, vk::SampleCountFlagBits::e1,
                  vk::AttachmentLoadOp::eClear,
                  vk::AttachmentStoreOp::eStore,
                  vk::AttachmentLoadOp::eDontCare,
                  vk::AttachmentStoreOp::eDontCare,
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::ImageLayout::eColorAttachmentOptimal
                 }
        };

        std::vector<vk::AttachmentReference> attachRef{{0, vk::ImageLayout::eColorAttachmentOptimal}};
        vk::SubpassDescription subpass{{}, vk::PipelineBindPoint::eGraphics, {}, attachRef, {}, {}};

        vk::RenderPassCreateInfo createInfo{{}, attachmentDes, subpass, {}};
        vkCreate([&](){ mOffscreenRenderPass = mDevice.createRenderPass(createInfo);}, " create off-screen render pass");

        return *this;
    }

    vk_base &vk_base::createOffscreenFrameBuffers() {
        for(auto& offFramebuffer : mOffscreenFrameBuffers){
            mDevice.destroy(offFramebuffer);
        }

        std::array<vk::ImageView, 1> attachments{};
        vk::FramebufferCreateInfo createInfo{{}, mOffscreenRenderPass, attachments, mExtent.width, mExtent.height, 1};

        mFrameBuffers.resize(mSwapchain.getImageCount());
        for(uint32_t i = 0; i < mSwapchain.getImageCount(); i++){
            attachments[0] = mSwapchain.getImageView(i);
            vkCreate([&](){ mFrameBuffers[i] = mDevice.createFramebuffer(createInfo);}, " create off-screen framebuffer " + std::to_string(i), 0);
        }

        return *this;
    }

    vk_base &vk_base::createRenderPass() {
        assert(mSwapchain.getSwapchain() != VK_NULL_HANDLE && "you must create swapchain before render pass, please call the create swapchain first. :)");
        std::vector<vk::AttachmentDescription> attachmentDescription{{{},
                                                                      mColorFormat, vk::SampleCountFlagBits::e1,
                                                                      vk::AttachmentLoadOp::eLoad,
                                                                      vk::AttachmentStoreOp::eStore,
                                                                      vk::AttachmentLoadOp::eDontCare,
                                                                      vk::AttachmentStoreOp::eDontCare,
                                                                      vk::ImageLayout::eColorAttachmentOptimal,
                                                                      vk::ImageLayout::eColorAttachmentOptimal},
                                                                     {{},
                                                                      mDepthFormat, vk::SampleCountFlagBits::e1,
                                                                      vk::AttachmentLoadOp::eClear,
                                                                      vk::AttachmentStoreOp::eStore,
                                                                      vk::AttachmentLoadOp::eDontCare,
                                                                      vk::AttachmentStoreOp::eDontCare,
                                                                      vk::ImageLayout::eUndefined,
                                                                      vk::ImageLayout::eDepthStencilAttachmentOptimal}};
        std::vector<vk::AttachmentReference> attachmentReference{{0, vk::ImageLayout::eColorAttachmentOptimal},
                                                                 {1, vk::ImageLayout::eDepthStencilAttachmentOptimal}};

        vk::SubpassDescription subpass{{}, vk::PipelineBindPoint::eGraphics, {}, attachmentReference[0], {}, &attachmentReference[1]};

        vk::RenderPassCreateInfo createInfo{{}, attachmentDescription, subpass, {}};
        vkCreate([&](){ mRenderPass = mDevice.createRenderPass(createInfo); }, "create renderPass");

        return *this;
    }

    vk_base &vk_base::createFrameBuffer() {
        for(auto& framebuffer : mFrameBuffers){
            mDevice.destroy(framebuffer);
        }

        std::array<vk::ImageView, 2> attachments{};
        vk::FramebufferCreateInfo createInfo{{}, mRenderPass, attachments,
                                             mExtent.width, mExtent.height, 1};

        mFrameBuffers.resize(mSwapchain.getImageCount());
        for(uint32_t i = 0; i < mSwapchain.getImageCount(); i++){
            attachments[0] = mSwapchain.getImageView(i);
            attachments[1] = mDepthView;
            vkCreate([&](){ mFrameBuffers[i] = mDevice.createFramebuffer(createInfo);}, "create framebuffer" + std::to_string(i), 0);
        }

        return *this;
    }

    vk_base &vk_base::prepareFrame() {
        int w, h;
        glfwGetFramebufferSize(Window::get()->window(), &w, &h);

        if (w != (int)mExtent.width || h != (int)mExtent.height){
            onFrameBufferSize(w, h);
        }
        if (!mSwapchain.acquire()){

        }

        uint32_t imageIndex = mSwapchain.getActiveImageIndex();

        if(vk::Result{ mDevice.waitForFences(1, &mWaitFences[imageIndex], VK_TRUE, UINT64_MAX)} != vk::Result::eSuccess){
            throw std::runtime_error("failed to wait fence\n");
        }

        return *this;
    }

    void vk_base::onFrameBufferSize(int w, int h) {
        if (w == 0 || h == 0)
            return;

        mDevice.waitIdle();
        mGraphicsQueue.waitIdle();

        mSwapchain.upDate(w, h, mVsync);
        mExtent = mSwapchain.getExtent();

        if (mExtent.width != w || mExtent.height != h){

        }

        createDepthBuffer();
        createFrameBuffer();
    }

    vk_base &vk_base::setViewport(const vk::CommandBuffer &cmd) {
        vk::Viewport viewport{0.f, 0.f, static_cast<float>(mExtent.width), static_cast<float>(mExtent.height), 0.f, 1.f};
        cmd.setViewport(0, viewport);

        vk::Rect2D scissor{{0, 0}, {mExtent.width, mExtent.height}};
        cmd.setScissor(0, scissor);

        return *this;
    }

    vk_base &vk_base::submitFrame(const std::vector<vk::CommandBuffer> &frontCmd,
                                  const std::vector<vk::CommandBuffer> &backCmd) {
        uint32_t imageIndex = mSwapchain.getActiveImageIndex();
        mDevice.resetFences(mWaitFences[imageIndex]);

        vk::Semaphore semaphoreRead = mSwapchain.getActiveReadSemaphore();
        vk::Semaphore semaphoreWritten = mSwapchain.getActiveWrittenSemaphore();

        std::vector<vk::PipelineStageFlags> waiteStage{vk::PipelineStageFlagBits::eColorAttachmentOutput};

        std::vector<vk::CommandBuffer> submitBuf{mCommandBuffers[imageIndex]};
        submitBuf.insert(submitBuf.begin(), frontCmd.begin(), frontCmd.end());
        submitBuf.insert(submitBuf.end(), backCmd.begin(), backCmd.end());
        vk::SubmitInfo submitInfo{semaphoreRead, waiteStage, submitBuf, semaphoreWritten};

        mGraphicsQueue.submit(submitInfo, mWaitFences[imageIndex]);
        mSwapchain.present(mGraphicsQueue);

        return *this;
    }



    vk::CommandBuffer vk_base::createTempCmdBuf() {
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, 1};
        vk::CommandBuffer cmd = mDevice.allocateCommandBuffers(allocateInfo).front();

        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        cmd.begin(beginInfo);

        return cmd;
    }

    void vk_base::submitTempCmdBuf(vk::CommandBuffer& cmd) {
        cmd.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmd);
        mGraphicsQueue.submit(submitInfo);
        mGraphicsQueue.waitIdle();
        mDevice.free(mCommandPool, cmd);
    }

    vk::CommandBuffer &vk_base::beginCmd() {
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

        uint32_t imageIndex = mSwapchain.getActiveImageIndex();
        mCommandBuffers[imageIndex].begin(beginInfo);
        {
            std::vector<vk::ClearValue> clearValues{{},
                                                    vk::ClearDepthStencilValue{1.f, 0}};

            vk::RenderPassBeginInfo renderPassBeginInfo{mRenderPass, mFrameBuffers[imageIndex],
                                                        {{0, 0}, mExtent}, clearValues};

            mCommandBuffers[imageIndex].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        }

        return mCommandBuffers[imageIndex];
    }

    void vk_base::endCmd(vk::CommandBuffer &cmd) {
        cmd.endRenderPass();
        cmd.end();
    }

    void vk_base::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, const vk::CommandBuffer& cmd, vk::Image image) {
        vk::ImageSubresourceRange imageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1};
        vk::ImageMemoryBarrier barrier{{}, {}, oldLayout, newLayout,
                                       vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
                                       imageSubresourceRange};
        vk::PipelineStageFlags srcStage, dstStage;
        if (oldLayout == vk::ImageLayout::eUndefined){
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eTransfer;
        } else {
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            srcStage = vk::PipelineStageFlagBits::eTransfer;
            dstStage = vk::PipelineStageFlagBits::eFragmentShader;
        }

        cmd.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);
    }

    bool vk_base::isMinimized(bool doSleeping) {
        int w, h;
        glfwGetWindowSize(Window::get()->window(), &w, &h);

        bool minimized{w == 0 || h == 0};
        if (minimized && doSleeping){
#ifdef _WIN32
            Sleep(50);
#else
            usleep(50);
#endif
        }
        return minimized;
    }

} // yic




























