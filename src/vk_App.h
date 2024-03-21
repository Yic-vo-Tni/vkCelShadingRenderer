//
// Created by lenovo on 11/25/2023.
//

#ifndef VULKAN_VK_APP_H
#define VULKAN_VK_APP_H

#include "vk_rhi.h"

namespace yic {

    class Application : public nonCopyable{
    public:
        Application();
        ~Application();

        void run();

    private:
        std::unique_ptr<RHI> rhi_ = std::make_unique<RHI>();

        vk::Instance instance{};
    };

} // yic

#endif //VULKAN_VK_APP_H









































//        double frameRate_{60.0}, alpha_{0.5};
//        double targetFrameTime = 1.0 / 60.0;
//        double accumulatedTime = 0.0;
//        double currentTime = glfwGetTime();
//        double lastFrameTime = currentTime;
//        void fixedFrameRate();