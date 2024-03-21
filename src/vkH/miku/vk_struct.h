//
// Created by lenovo on 2/4/2024.
//

#ifndef VKMMD_VK_STRUCT_H
#define VKMMD_VK_STRUCT_H

#include "yic_pch.h"

namespace yic{

    struct imageState{
        vk::ImageCreateInfo createInfo{{}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm, vk::Extent3D{mExtent, 1},
                                       1, 1, vk::SampleCountFlagBits::e1,
                                       vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive};

        imageState(vk::Device device, vk::Extent2D extent) : mExtent(extent){
            vkCreate([&](){device.createImage(createInfo);}, "create image");
        };

    private:
        vk::Extent2D mExtent;
    };

}

#endif //VKMMD_VK_STRUCT_H
