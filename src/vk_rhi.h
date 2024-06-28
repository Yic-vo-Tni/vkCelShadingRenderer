//
// Created by lenovo on 12/18/2023.
//

#ifndef VULKAN_VK_RHI_H
#define VULKAN_VK_RHI_H

#include "vkInit/vk_init.h"
#include "vkContext/vkRayTracing.h"

#include "vk_context.h"
#include "editor/vkImgui_context.h"

#include "json/baseSetting.h"

namespace yic {

    class RHI : public nonCopyable{
    public:
        RHI();
        ~RHI();

        void drawFrame();
    private:
        void initVulkan();

        vk_context vkContext{};
        vkRayTracing mvkRayTracing{};
        std::unique_ptr<ImRender::vkImgui_context> vkImContext;

    private:
        std::thread mThread;
        std::streambuf *mOldCoutBuf{};

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
        std::chrono::time_point<std::chrono::high_resolution_clock> mEnd;
        std::chrono::milliseconds mDuration{};
        bool mOneTimes{};
    };

} // yic

#endif //VULKAN_VK_RHI_H
