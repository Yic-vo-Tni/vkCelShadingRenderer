//
// Created by lenovo on 12/18/2023.
//

#ifndef VULKAN_RHI_CO_H
#define VULKAN_RHI_CO_H

#include "log/Log.h"

namespace yic {

    struct queueFamilyIndices{
        std::optional<uint32_t> graphicsFamily{~0};
        std::optional<uint32_t> presentFamily{0xFFFFFFFF};
        std::optional<uint32_t> transferFamily;

        explicit operator bool() const{
            return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
        }

        static queueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surfaceKhr);
    };




} // yic

#endif //VULKAN_RHI_CO_H
