//
// Created by lenovo on 12/20/2023.
//

#include "vk_fn.h"

namespace yic {

    vk::ImageView vk_fn::createImageView(vk::Device device, vk::Image image, vk::Format format,
                                         vk::ImageAspectFlags flags) {
        vk::ImageViewCreateInfo createInfo{{}, image, vk::ImageViewType::e2D, format, vk::ComponentSwizzle::eIdentity,
                                           vk::ImageSubresourceRange(flags, 0, 1, 0, 1)};
        vk::ImageView imageView{device.createImageView(createInfo)};

        return imageView;
    }

    uint32_t vk_fn::getMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeBites,
                                  const vk::MemoryPropertyFlags &propertyFlags) {
        vk::PhysicalDeviceMemoryProperties memPros{physicalDevice.getMemoryProperties()};

        for(uint32_t i = 0; i < memPros.memoryTypeCount; i++){
            if (((typeBites & (1 << i)) != 0) && (memPros.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags){
                return i;
            }
        }
        vkError("failed to find memory type");
        return ~0u;
    }

    vk::CommandPool vk_fn::getCommandPool(vk::Device device, vk::CommandPoolCreateFlags flags, uint32_t queueIndex) {
        auto createInfo = vk::CommandPoolCreateInfo().setFlags(flags)
                .setQueueFamilyIndex(queueIndex);
        return vk::CommandPool{device.createCommandPool(createInfo)};
    }

    bool vk_fn::getDepthFormat(vk::PhysicalDevice physicalDevice, vk::Format& format) {
        if (format == vk::Format::eUndefined){
            auto feature {vk::FormatFeatureFlagBits::eDepthStencilAttachment};
            for(const auto& f : {vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint}){
                vk::FormatProperties formatProperties{};
                physicalDevice.getFormatProperties(f, &formatProperties);
                if ((formatProperties.optimalTilingFeatures & feature) == feature){
                    format = f;
                    return true;
                }
            }
        }
        return false;
    }

} // yic