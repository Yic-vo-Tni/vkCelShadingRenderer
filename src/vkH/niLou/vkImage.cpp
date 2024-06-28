//
// Created by lenovo on 1/2/2024.
//

#include "vkImage.h"

namespace yic {

    vkImage::vkImage(const std::string &filePath, const bool &def) {
        if (def)
            createTempCmdBuf();

        loadTexture(filePath);
        standardProcess();

        if (def)
            submitTempCmdBuf();
    }


    vkImage &vkImage::loadTexture(const std::string &filePath) {
        int w, h, channels;

        saba::File file;
        if (!file.Open(filePath)) { std::cout << "Failed to open file.[" << filePath << "]\n"; }
        if (stbi_info_from_file(file.GetFilePointer(), &w, &h, &channels) == 0) { std::cout << "Failed to read file.\n"; }
        stbi_uc *pixels = stbi_load_from_file(file.GetFilePointer(), &w, &h, &channels, STBI_rgb_alpha);

        if (channels == 4) { mHasAlpha = true; }
        if (pixels) {
            size_t imageSize = w * h * 4;
            mImageSize += imageSize;
            mExtent = vk::Extent2D{(uint32_t)w, (uint32_t)h};
            mPixels.insert(mPixels.end(), pixels, pixels + imageSize);
            stbi_image_free(pixels);
        } else { vkDebug{ std::cerr << "failed to load tex"; } }

        return *this;
    }

    vkImage &vkImage::standardProcess() {
        createImage();
        createSampler();
        allocBuffer(mImageSize, mPixels.data(), vk::BufferUsageFlagBits::eTransferSrc, memReqs, bindMem, memFlags);

        transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        copyBuffer2Image();
        mPixels.clear();

        //transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        //createImageView();

        return *this;
    }


    vkImage &vkImage::createImage() {
        imageCreateInfo.setExtent({mExtent.width, mExtent.height, 1});
        vkCreate([&](){mImage.image = mDevice.createImage(imageCreateInfo);}, "create image");

        return *this;
    }

    vkImage &vkImage::createSampler() {
        vkCreate([&](){ mImage.sampler = mDevice.createSampler(samplerCreateInfo);}, "create sampler");

        return *this;
    }

    vkImage &vkImage::createImageView() {
        imageViewCreateInfo.setImage(mImage.image);
        vkCreate([&](){mImage.imageView = mDevice.createImageView(imageViewCreateInfo);}, "create image view");

        return *this;
    }

    vkImage &vkImage::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, const vk::CommandBuffer& cmd) {
        vk::ImageMemoryBarrier barrier{{}, {}, oldLayout, newLayout,
                                       vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, mImage.image,
                                       imageSubresourceRange};
        vk::PipelineStageFlags srcStage, dstStage;
        if (oldLayout == vk::ImageLayout::eUndefined){
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eTransfer;
        } else {
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            srcStage = vk::PipelineStageFlagBits::eTransfer;
            dstStage = vk::PipelineStageFlagBits::eFragmentShader;
        }

        if (cmd)
            cmd.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);
        else
            mCmd.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);

        return *this;
    }

    vkImage &vkImage::configureImageForRender(const vk::CommandBuffer& cmd) {
        if (!mInitOneTime){
            transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, cmd);
            createImageView();
            mInitOneTime = true;
        }

        return *this;
    }

    vkImage &vkImage::copyBuffer2Image() {
        vk::BufferImageCopy copy{0, 0, 0,
                                 imageSubresourceLayers,
                                 {0, 0, 0}, vk::Extent3D{mExtent, 1}};

        mCmd.copyBufferToImage(mStagingBuffer.buffer, mImage.image, vk::ImageLayout::eTransferDstOptimal, copy);

        return *this;
    }

} // yic