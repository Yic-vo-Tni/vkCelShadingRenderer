//
// Created by lenovo on 1/5/2024.
//

#ifndef VULKAN_VKIMGUI_CONTEXT_H
#define VULKAN_VKIMGUI_CONTEXT_H

#include "ke_q/vk_allocator.h"
#include "ke_q/file_operation.h"

#include "niLou/vkSwapchain.h"
#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"
#include "niLou/vkImage.h"

#include "editor/vkImgui.h"

#include "vkInit/vk_init.h"

#include "centralizedTaskSystem/vkCommon.h"

namespace ImRender {

    using namespace yic;

    class vkImgui_context : public nonCopyable{
    public:
        explicit vkImgui_context(vk_init* vkInit, vkSwapchain& swapchain);
        ~vkImgui_context(){ mDevice.waitIdle(); mvkImgui.reset(); }

        vkImgui_context& renderImgui();

        vkImgui_context& recreate();
        vk::CommandBuffer& getActiveCmdBuf() {return mCommandBuffers[mSwapchain.getActiveImageIndex()];};
    private:
        vkImgui_context& createFrameBuffer();
        vkImgui_context& createRenderPass();
        vkImgui_context& createGraphicsPipeline();
        vkImgui_context& taskAssignment();
        vkImgui_context& updateDescriptorSet();

    private:
        vk::CommandBuffer createTempCmdBuf();
        void submitTempCmdBuf(vk::CommandBuffer& cmd);

        /// config
        vk::Instance mInstance;
        vk::PhysicalDevice mPhysicalDevice;
        vk::Device mDevice;
        uint32_t mGraphicsIndex{};
        vk::Queue mGraphicsQueue;

        vkSwapchain& mSwapchain;

        vk::Extent2D mExtent;
        std::vector<vk::Framebuffer> mFrameBuffers;

        vk::RenderPass mRenderPass;
        vk::Pipeline mGraphicsPipeline;
        vk::PipelineLayout mPipelineLayout;

        vkDescriptor mDescriptor{};
        vk::CommandPool mCommandPool;
        std::vector<vk::CommandBuffer> mCommandBuffers;

        /// imgui
        std::unique_ptr<vkImgui> mvkImgui;
        // tex
        //genericTexManagerUptr mImagePng2;
        genericTexManagerSptr mImagePng2;
    };

} // lmRender

#endif //VULKAN_VKIMGUI_CONTEXT_H
