//
// Created by lenovo on 1/2/2024.
//

#ifndef VULKAN_VKIMAGE_H
#define VULKAN_VKIMAGE_H

#include "ke_q/vk_allocator.h"

#include "miku/vk_fn.h"
#include <Saba/Base/File.h>

namespace yic {

    struct textureState{
        vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

        vk::ImageSubresourceRange imageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
        vk::ImageSubresourceLayers imageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1};

        vk::ImageCreateInfo imageCreateInfo{{}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm, vk::Extent3D{},
                                           1, 1, vk::SampleCountFlagBits::e1,vk::ImageTiling::eOptimal,
                                           vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive};
        vk::SamplerCreateInfo samplerCreateInfo{{}, vk::Filter::eLinear, vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear,
                                         vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                                         0.f, vk::False, 1.f,
                                         vk::False, vk::CompareOp::eAlways,
                                         0.f, 0.f,
                                         vk::BorderColor::eIntOpaqueBlack, vk::False};
        vk::ImageViewCreateInfo imageViewCreateInfo{{}, {}, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm,
                                                    vk::ComponentSwizzle::eIdentity, imageSubresourceRange};
    protected:
        Image mImage;
    private:

    };

    class vkImage : public vkAllocator, public textureState{
    public:
        vkImage() = default;
        ~vkImage() { mDevice.destroy(mStagingBuffer.buffer); mDevice.free(mStagingBuffer.deviceMemory); }
        explicit vkImage(const std::string& filePath, const bool& def = true);
    protected:
        MemReqs memReqs = [this](){ return vk::MemoryRequirements{mDevice.getImageMemoryRequirements(mImage.image)};};
        BindMem bindMem = [this](){ mDevice.bindImageMemory(mImage.image, mBuffer.deviceMemory, 0);};

        vkImage& loadTexture(const std::string& filePath);
        vkImage& standardProcess();
    private:
        vkImage& createImage();
        vkImage& createSampler();
        vkImage& createImageView();
        vkImage& transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, const vk::CommandBuffer& cmd = {});
        vkImage& copyBuffer2Image();


    public:
        vkImage& configureImageForRender(const vk::CommandBuffer& cmd);

    public:
        [[nodiscard]] inline auto& getImage() const  { return mImage.image;}
        [[nodiscard]] inline auto& getSampler() const { return mImage.sampler;}
        [[nodiscard]] inline auto& getImageView() const { return mImage.imageView;}
        [[nodiscard]] inline auto& getImageSize() const { return mExtent;}
        [[nodiscard]] inline auto& hasAlpha() const { return mHasAlpha;}
    private:
        vk::DeviceSize mImageSize{};
        vk::Extent2D mExtent;
        bool mHasAlpha{false};
        std::vector<stbi_uc> mPixels{};
        bool mInitOneTime{false};
    };

    class vkCubeMap : public vkImage{
    public:
        explicit vkCubeMap(const std::vector<std::string>& filePaths) : vkImage(){
            imageSubresourceRange.setLayerCount(6);
            imageSubresourceLayers.setLayerCount(6);

            imageCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible)
                    .setArrayLayers(6);
            imageViewCreateInfo.setViewType(vk::ImageViewType::eCube)
                    .setSubresourceRange(imageSubresourceRange);

            for(auto& filePath : filePaths){
                loadTexture(filePath);
            }

            createTempCmdBuf();
            standardProcess();
            submitTempCmdBuf();
        }

    private:

    };

} // yic

    using genericTexManagerUptr = std::unique_ptr<yic::vkImage>;
    using genericTexManagerSptr = std::shared_ptr<yic::vkImage>;

    using genericSkyboxManagerUptr = std::unique_ptr<yic::vkCubeMap>;
    using genericSkyboxManagerSptr = std::shared_ptr<yic::vkCubeMap>;



#endif //VULKAN_VKIMAGE_H
