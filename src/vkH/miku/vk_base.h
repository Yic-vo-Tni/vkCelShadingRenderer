//
// Created by lenovo on 12/19/2023.
//

#ifndef VULKAN_VK_BASE_H
#define VULKAN_VK_BASE_H

#include "niLou/vkSwapchain.h"
#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"

#include "editor/vkImgui.h"

#include "ke_q/file_operation.h"
#include "ke_q/vk_allocator.h"

#include "vkInit/vk_window.h"

namespace yic {

    class vk_base {
    public:
        ~vk_base() {mDevice.waitIdle();};

    protected:
        vk_base& setup(vk_init* vkInit);

    public:
        vk_base& createSwapchain(const vk::SurfaceKHR& surface, Window* window,
                vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm, vk::Format depthFormat = vk::Format::eUndefined);
        vk_base& createDepthBuffer();
        virtual vk_base& createRenderPass();
//        virtual vk_base& createGraphicsPipeline() = 0;
//        virtual vk_base& updateDescriptorSet() = 0;
        vk_base& createFrameBuffer();
        vk_base& prepareFrame();
        vk_base& setViewport(const vk::CommandBuffer& cmd);
        vk_base& submitFrame(const std::vector<vk::CommandBuffer>& frontCmd = {}, const std::vector<vk::CommandBuffer>& backCmd = {});

        vk::CommandBuffer createTempCmdBuf();
        void submitTempCmdBuf(vk::CommandBuffer& cmd);
        vk::CommandBuffer& beginCmd();
        void endCmd(vk::CommandBuffer& cmd);

        static bool isMinimized(bool doSleeping = true);

        [[nodiscard]] inline auto& getSwapchain() const { return mSwapchain;}
        [[nodiscard]] inline auto& getSwapchain()  { return mSwapchain;}
        [[nodiscard]] inline auto& getRenderPass() const { return mRenderPass;}
        [[nodiscard]] inline auto& getRenderPass()  { return mRenderPass;}
        [[nodiscard]] inline auto& getFrameBuffers() const { return mFrameBuffers;}
        [[nodiscard]] inline auto& getFrameBuffers()  { return mFrameBuffers;}
        [[nodiscard]] inline auto& getColorFormat() const { return mColorFormat;}
        [[nodiscard]] inline auto& getDepthFormat() const { return mDepthFormat;}
        [[nodiscard]] inline auto& getDepthMem() const { return mDepthMemory;}
        [[nodiscard]] inline auto& getDepthImage() const { return mDepthImage;}
        [[nodiscard]] inline auto& getDepthImageView() const { return mDepthView;}
        [[nodiscard]] inline auto& getCommandPool() const { return mCommandPool;}
        [[nodiscard]] inline auto& getCmdBuffer() const { return mCommandBuffers;}
        [[nodiscard]] inline auto& getFences() const { return mWaitFences;}
    protected:
        vk::Device mDevice{};
        vk::PhysicalDevice mPhysicalDevice{};

        vk::Queue mGraphicsQueue{};
        uint32_t mGraphicsIndex{(uint32_t)-1};

        vk::Format mColorFormat{};
        vk::Format mDepthFormat{};

        vk::Extent2D mExtent{0, 0};
        vkSwapchain mSwapchain{};
        vk::CommandPool mCommandPool{};

        vk::RenderPass mRenderPass{VK_NULL_HANDLE};

        std::vector<vk::Framebuffer> mFrameBuffers;
        std::vector<vk::CommandBuffer> mCommandBuffers;
        std::vector<vk::Fence> mWaitFences;

        vk::Image mDepthImage{VK_NULL_HANDLE};
        vk::DeviceMemory mDepthMemory{VK_NULL_HANDLE};
        vk::ImageView mDepthView{VK_NULL_HANDLE};

        bool mVsync{false};

        vkDescriptor mDesc{};
    private:
        void onFrameBufferSize(int w, int h);
    };

} // yic

#endif //VULKAN_VK_BASE_H
