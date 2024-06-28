//
// Created by lenovo on 4/14/2024.
//

#include "renderDepth.h"

namespace yic {

    renderDepth::renderDepth(yic::vk_init *vkInit, yic::vkSwapchain &swapchain, vk::ImageView& depthRenderImageView) : mSwapchain{swapchain}, mDepthRenderImageView(depthRenderImageView) {
            mDevice = vkInit->device();
            mPhysicalDevice = vkInit->physicalDevice();
            mExtent = mSwapchain.getExtent();

            createRenderDepthImage();
            createRenderDepthImageView();
            createRenderDepthSampler();
            createRenderPass();
            createPipeline();
            createFrameBuffers();
            updateDescriptor();

        vk::CommandPoolCreateInfo createInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, vkInit->getGraphicsFamilyIndices()};
        vkCreate([&](){ mCommandPool = mDevice.createCommandPool(createInfo); }, " create depth command pool ");

        mCommandBuffers.resize(mSwapchain.getImageCount());
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, mSwapchain.getImageCount()};
        vkCreate([&](){ mCommandBuffers = mDevice.allocateCommandBuffers(allocateInfo); }, "allocate depth commandBuffers");
    }

    bool renderDepth::createRenderDepthImage() {
        imageCreateInfo.setExtent({mExtent.width, mExtent.height, 1});
        imageCreateInfo.usage |= vk::ImageUsageFlagBits::eColorAttachment;
        vkCreate([&](){mImage.image = mDevice.createImage(imageCreateInfo);}, "create depth image");

        vk::MemoryRequirements memReqs{mDevice.getImageMemoryRequirements(mImage.image)};
        vk::MemoryAllocateInfo allocateInfo{memReqs.size, vk_fn::getMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
        vkCreate([&](){ mDepthMemory = mDevice.allocateMemory(allocateInfo); }, "allocate depth memory");
        //
        mDevice.bindImageMemory(mImage.image, mDepthMemory, 0);

        return true;
    }

    bool renderDepth::createRenderDepthImageView() {
        imageViewCreateInfo.setImage(mImage.image);
        vkCreate([&](){mImage.imageView = mDevice.createImageView(imageViewCreateInfo);}, "create depth image view");

        return true;
    }

    bool renderDepth::createRenderDepthSampler() {
        vkCreate([&](){ mImage.sampler = mDevice.createSampler(samplerCreateInfo);}, "create depth sampler");

        return true;
    }

    bool renderDepth::createFrameBuffers() {
        for(auto& framebuffer : mFrameBuffers){
            mDevice.destroy(framebuffer);
        }

        std::array<vk::ImageView, 1> attachments{};
        vk::FramebufferCreateInfo createInfo{{}, mRenderPass, attachments,
                                             mExtent.width, mExtent.height, 1};

        mFrameBuffers.resize(mSwapchain.getImageCount());
        for(uint32_t i = 0; i < mSwapchain.getImageCount(); i++){
            attachments[0] = mImage.imageView;
            vkCreate([&](){ mFrameBuffers[i] = mDevice.createFramebuffer(createInfo);}, "create framebuffer" + std::to_string(i), 0);
        }

        return true;
    }

    bool renderDepth::createRenderPass() {
        std::vector<vk::AttachmentDescription> attachmentDescription{{{},
                                                                             imageCreateInfo.format, vk::SampleCountFlagBits::e1,
                                                                             vk::AttachmentLoadOp::eClear,
                                                                             vk::AttachmentStoreOp::eStore,
                                                                             vk::AttachmentLoadOp::eDontCare,
                                                                             vk::AttachmentStoreOp::eDontCare,
                                                                             vk::ImageLayout::eUndefined,
                                                                             vk::ImageLayout::eShaderReadOnlyOptimal}};
        std::vector<vk::AttachmentReference> attachmentReference{{0, vk::ImageLayout::eColorAttachmentOptimal}};

        vk::SubpassDescription subpass{{}, vk::PipelineBindPoint::eGraphics, {}, attachmentReference, {}, {}};

        vk::RenderPassCreateInfo createInfo{{}, attachmentDescription, subpass, {}};
        vkCreate([&](){ mRenderPass = mDevice.createRenderPass(createInfo); }, "create renderPass");

        return true;
    }

    bool renderDepth::createPipeline() {
        mDescriptor.addDescriptorSetLayout({
                                                   vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                           });
        mRenderDepthDescriptor.addDescriptorSetLayout({
                                                   vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                           });

        mPipelineLayout = mDescriptor.getPipelineLayout();
        yic::graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mDescriptor.getPipelineLayout(), mRenderPass};
        pipelineCombined.addShader(ke_q::loadFile("v_depth.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_depth.spv"), vk::ShaderStageFlagBits::eFragment);
        mPipeline = pipelineCombined.createGraphicsPipeline();


        return true;
    }


    bool renderDepth::updateDescriptor() {
        mDescriptor.increaseMaxSets()
                .createDescriptorPool()
                .createDescriptorSets()
                .addDescriptorSet({
                    vk::DescriptorImageInfo{mImage.sampler, mDepthRenderImageView, vk::ImageLayout::eShaderReadOnlyOptimal}
                })
                .update();
        mRenderDepthDescriptor.increaseMaxSets()
                .createDescriptorPool()
                .createDescriptorSets()
                .addDescriptorSet({
                                          vk::DescriptorImageInfo{mImage.sampler, mImage.imageView, vk::ImageLayout::eShaderReadOnlyOptimal}
                                  })
                .update();

        if (!vkImguiManager::get()->getImguiTask(eView).findTask(mDepthImageRender)) {
            mDepthImageRender = vkImguiManager::get()->addRenderToView([this]() {

                ImVec2 windowSize = ImGui::GetWindowSize();
                ImVec2 imageSize = ImVec2{(float)Window::get()->width() * (3.f / 4.f), (float)Window::get()->height()};
                float aspectRatio = imageSize.x / imageSize.y;
                ImVec2 displaySize = windowSize;

                if (windowSize.x / windowSize.y > aspectRatio) {
                    displaySize.x = windowSize.y * aspectRatio;
                } else {
                    displaySize.y = windowSize.x / aspectRatio;
                }

                ImGui::Image((ImTextureID) mRenderDepthDescriptor.getDescriptorSets().front(), displaySize, ImVec2{0.f, 0.f}, ImVec2{3.f / 4.f, 1.f});
            });
        }

        return true;
    }

    bool renderDepth::renderDepthImage() {
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        std::vector<vk::ClearValue> clearValues{vk::ClearColorValue{0.2f, 0.3f, 0.2f, 1.f}};
        uint32_t imageIndex = mSwapchain.getActiveImageIndex();
        auto& cmd = mCommandBuffers[imageIndex];
        cmd.begin(beginInfo);
        {
            vk::RenderPassBeginInfo renderPassBeginInfo{mRenderPass, mFrameBuffers[imageIndex],
                                                        {{0, 0}, mExtent}, clearValues};

            {
                cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

                vk::Viewport viewport{0.f, 0.f, static_cast<float>(mExtent.width), static_cast<float>(mExtent.height), 0.f, 1.f};
                cmd.setViewport(0, viewport);

                vk::Rect2D scissor{{0, 0}, {mExtent.width, mExtent.height}};
                cmd.setScissor(0, scissor);

                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, mDescriptor.getDescriptorSets(), nullptr);

                cmd.draw(6, 1, 0, 0);
            }
            cmd.endRenderPass();
        }
        cmd.end();

        return true;
    }


} // yic