//
// Created by lenovo on 4/14/2024.
//

#ifndef VKMMD_RENDERDEPTH_H
#define VKMMD_RENDERDEPTH_H

#include "niLou/vkSwapchain.h"
#include "niLou/vkImage.h"
#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"

#include "ke_q/file_operation.h"
#include "editor/vkImguiTaskQueue.h"

namespace yic {

    class renderDepth : public textureState{
    public:
        explicit renderDepth(vk_init* vkInit, vkSwapchain& swapchain, vk::ImageView& depthRenderImageView);

        bool renderDepthImage();
        bool updateDescriptor();

        vk::CommandBuffer& getActiveCmdBuf() {return mCommandBuffers[mSwapchain.getActiveImageIndex()];};
    private:
        bool createRenderDepthImage();
        bool createRenderDepthImageView();
        bool createRenderDepthSampler();
        bool createFrameBuffers();
        bool createRenderPass();
        bool createPipeline();

    private:
        vk::Device mDevice;
        vk::PhysicalDevice mPhysicalDevice;
        vk::Extent2D mExtent;
        vkSwapchain& mSwapchain;

        vk::RenderPass mRenderPass{};
        vk::Pipeline mPipeline{};
        vk::PipelineLayout mPipelineLayout{};
        vkDescriptor mDescriptor{};
        vkDescriptor mRenderDepthDescriptor{};
        vk::ImageView mDepthRenderImageView{};
        vk::DeviceMemory mDepthMemory;

        vk::CommandPool mCommandPool;
        std::vector<vk::Framebuffer> mFrameBuffers{};
        std::vector<vk::CommandBuffer> mCommandBuffers{};
        int mDepthImageRender{-1};
    };

} // yic

#endif //VKMMD_RENDERDEPTH_H
