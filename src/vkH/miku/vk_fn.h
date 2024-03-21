//
// Created by lenovo on 12/20/2023.
//

#ifndef VULKAN_VK_FN_H
#define VULKAN_VK_FN_H

#include "log/Log.h"

namespace yic{

    struct vk_fn {
      [[nodiscard]] static vk::ImageView createImageView(vk::Device device,
                                           vk::Image image,
                                           vk::Format format,
                                           vk::ImageAspectFlags flags = vk::ImageAspectFlagBits::eColor);

      [[nodiscard]] static uint32_t getMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeBites, const vk::MemoryPropertyFlags& propertyFlags) ;

      [[nodiscard]] static vk::CommandPool getCommandPool(vk::Device device, vk::CommandPoolCreateFlags flags, uint32_t queueIndex);

      static bool getDepthFormat(vk::PhysicalDevice physicalDevice, vk::Format& format);

    };
}

#endif //VULKAN_VK_FN_H
