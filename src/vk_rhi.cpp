//
// Created by lenovo on 12/18/2023.
//

#include "vk_rhi.h"

namespace yic {

    RHI::RHI() {
        mStart = std::chrono::high_resolution_clock::now();
        initVulkan();
    }

    RHI::~RHI() {
        vkImContext.reset();
    };

    void RHI::initVulkan() {
        /// glfw window
        Window::get(1200, 800, "Yicvot")->glfwCallback();

        /// vk init
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{};
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtFeature{};
        contextCreateInfo createInfo{};
        createInfo.addInstanceLayers("VK_LAYER_KHRONOS_validation")
                .addInstanceExtensions("VK_EXT_debug_utils")
                .addPhysicalDeviceExtensions("VK_KHR_swapchain")
                .addPhysicalDeviceExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelFeature)
                .addPhysicalDeviceExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rtFeature)
                .addPhysicalDeviceExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

        vk_init::get(createInfo)->oneTimeInitialization();
        // main
        vkContext.init()
                 .createSwapchain(vk_init::get()->surfaceKhr(), Window::get())
                 .createDepthBuffer()
                 .createRenderPass()
                 .createFrameBuffer();

        vkContext.prepareEveryContext();

        /// imgui
        vkImContext = std::make_unique<ImRender::vkImgui_context>(vk_init::get(), vkContext.getSwapchain());

        tasker::wQueueFactory::get()->execute(vkTaskGroupType::eResourceLoadGroup);

        taskerBuilder::executor::executeTaskflow();

    }

    void RHI::drawFrame() {
        // glfw
        Window::get()->calculateTime();
        // vk
        vkContext.updateEveryFrame();

        vkImContext->recreate();
        {
            auto cmd = vkContext.beginCmd();
            vkContext.rasterization(cmd);
            vkContext.endCmd(cmd);

            vkContext.orRender();

            vkImContext->renderImgui();
        }
        vkContext.submitFrame({vkContext.getActiveFrontCmdBuf()},
                              {vkImContext->getActiveCmdBuf()});

        if (!mOneTimes){
            mEnd = std::chrono::high_resolution_clock::now();
            mDuration = std::chrono::duration_cast<std::chrono::milliseconds>(mEnd - mStart);
            std::cout << mDuration.count() << " milliseconds. " << std::endl;
            mOneTimes = true;
        }
    }

} // yic