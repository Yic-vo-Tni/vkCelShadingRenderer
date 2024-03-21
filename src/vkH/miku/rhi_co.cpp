//
// Created by lenovo on 12/18/2023.
//

#include "rhi_co.h"

namespace yic {

    queueFamilyIndices queueFamilyIndices::findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surfaceKhr) {
        queueFamilyIndices indices{};
        auto queueFamilies{physicalDevice.getQueueFamilyProperties()};
        vkDebug{ vkTrance("physical device support {0} queue families! ", queueFamilies.size()); }
        for (const auto & queueFamily : queueFamilies) {
            std::cout << "\tQueue Flags: ";

            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                std::cout << "Graphics ";
            }
            if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                std::cout << "Compute ";
            }
            if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
                std::cout << "Transfer ";
            }
            if (queueFamily.queueFlags & vk::QueueFlagBits::eSparseBinding) {
                std::cout << "Sparse Binding ";
            }
            if (queueFamily.queueFlags & vk::QueueFlagBits::eProtected) {
                std::cout << "Protected ";
            }

            std::cout << "\n";
        }

        for(size_t i = 0; auto& queueFamily : queueFamilies){
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics){
                indices.graphicsFamily = i;
                vkDebug{ vkInfo("queue family {0} is suitable for graphics! ", i); };
            }
            if (physicalDevice.getSurfaceSupportKHR(i, surfaceKhr)){
                indices.presentFamily = i;
                vkDebug{ vkInfo("queue family {0} is suitable for present! ", i); };
            }
            if ((queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && !(queueFamily.queueFlags & vk::QueueFlagBits::eCompute)){
                indices.transferFamily = i;
                vkDebug{ vkInfo("queue family {0} is suitable for transfer! ", i); };
            }
            if (indices) break;

            i++;
        }

        return indices;
    }

} // yic