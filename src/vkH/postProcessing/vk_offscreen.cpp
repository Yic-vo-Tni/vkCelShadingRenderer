//
// Created by lenovo on 3/22/2024.
//


#include "vk_offscreen.h"

namespace yic {

    vkOffScreen &vkOffScreen::createOffScreenImage() {
        imageCreateInfo.setExtent({mExtent.width, mExtent.height, 1});
        imageCreateInfo.usage |= vk::ImageUsageFlagBits::eColorAttachment;
        vkCreate([&](){mImage.image = mDevice.createImage(imageCreateInfo);}, "create depth image");

        vk::MemoryRequirements memReqs{mDevice.getImageMemoryRequirements(mImage.image)};
        vk::MemoryAllocateInfo allocateInfo{memReqs.size, vk_fn::getMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
        vkCreate([&](){ mOffMemory = mDevice.allocateMemory(allocateInfo); }, "allocate depth memory");
        //
        mDevice.bindImageMemory(mImage.image, mOffMemory, 0);

        if (mDepthImageView)
            mDevice.destroy(mDepthImageView);
        if (mDepthImage)
            mDevice.destroy(mDepthImage);
        if (mDepthImageMemory)
            mDevice.free(mDepthImageMemory);

        vk::ImageCreateInfo createInfo{{},
                                       vk::ImageType::e2D, mDepthFormat, vk::Extent3D{mExtent, 1},
                                       1, 1, vk::SampleCountFlagBits::e1, {},
                                       vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled};
        vkCreate([&](){ mDepthImage = mDevice.createImage(createInfo); }, "create depth image");

        ///
        vk::MemoryRequirements memDepthReqs{mDevice.getImageMemoryRequirements(mDepthImage)};
        vk::MemoryAllocateInfo allocateDepthInfo{memDepthReqs.size, vk_fn::getMemoryType(mPhysicalDevice, memDepthReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
        vkCreate([&](){ mDepthImageMemory = mDevice.allocateMemory(allocateDepthInfo); }, "allocate depth memory");
        //
        mDevice.bindImageMemory(mDepthImage, mDepthImageMemory, 0);

        return *this;
    }

    vkOffScreen &vkOffScreen::createOffScreenImageView() {


        return *this;
    }

} // yic