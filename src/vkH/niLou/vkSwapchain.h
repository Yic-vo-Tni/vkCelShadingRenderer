//
// Created by lenovo on 12/19/2023.
//

#ifndef VULKAN_VKSWAPCHAIN_H
#define VULKAN_VKSWAPCHAIN_H

#include "nonCopyable.h"
#include "log/Log.h"
#include "miku/vk_fn.h"

namespace yic {

    struct swapChainAcquireState
    {
        VkImage     image;
        VkImageView view;
        uint32_t    index;
        VkSemaphore waitSem;
        VkSemaphore signalSem;
    };

    class vkSwapchain : public nonCopyable{
    public:
        static constexpr vk::ImageUsageFlags defaultFlags =
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst;

        struct Entry{
            vk::Image image{};
            vk::ImageView imageView{};
            vk::Semaphore readSemaphore{};
            vk::Semaphore writtenSemaphore{};
        };

        virtual vkSwapchain& init(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Queue queue,
                                  uint32_t queueFamilyIndex, vk::SurfaceKHR surfaceKhr,
                                  vk::Format format = vk::Format::eR8G8B8A8Unorm,
                                  vk::ImageUsageFlags imageUsageFlags = defaultFlags);

        virtual vkSwapchain& upDate(uint32_t width, uint32_t height, bool vsync);
        virtual vkSwapchain& upDate(uint32_t width, uint32_t height) { return upDate(width, height, mVsync); }

        bool acquireCustom(vk::Semaphore argSemaphore, int width, int height, bool *pRecreated, swapChainAcquireState* pOut);
        bool acquire(bool *pRecreated = nullptr, swapChainAcquireState* pOut = nullptr);

        [[nodiscard]] auto& getSurfaceFormat() const { return mSurfaceFormat;}
        [[nodiscard]] auto& getSwapchain() const {  return mSwapchain;}
        [[nodiscard]] auto& getImageCount() const { return mImageCount;}
        [[nodiscard]] auto& getExtent() const { return mExtent;}
        [[nodiscard]] auto& getActiveImageIndex() const { return mCurrentImage;}

        [[nodiscard]] inline auto& getCurrentImage() const { return mEntries[mCurrentImage].image;}
        [[nodiscard]] vk::Image getImage(uint32_t i) const { return mEntries[i].image; }
        [[nodiscard]] vk::ImageView getImageView(uint32_t i) const { if (i >= mImageCount) { return nullptr;} return mEntries[i].imageView; }
        [[nodiscard]] auto& getActiveReadSemaphore() const { return mEntries[(mCurrentSemaphore % mImageCount)].readSemaphore;}
        [[nodiscard]] auto& getActiveWrittenSemaphore() const { return mEntries[(mCurrentSemaphore % mImageCount)].writtenSemaphore;}

        [[nodiscard]] inline auto& getEntries() const { return mEntries;}
    private:
        vk::Device mDevice{VK_NULL_HANDLE};
        vk::PhysicalDevice mPhysicalDevice{VK_NULL_HANDLE};
        vk::Extent2D mExtent{0, 0};
        vk::SwapchainKHR mSwapchain{};
        vk::SurfaceKHR mSurface{};
        vk::Queue mQueue{};

        vk::Format mSurfaceFormat;
        vk::ColorSpaceKHR mColorSpace;

        uint32_t mQueueFamilyIndex{0};
        vk::ImageUsageFlags mImageUsage;

        std::vector<Entry> mEntries{};
        uint32_t mImageCount{0};

        uint32_t mCurrentImage{0};
        uint32_t mCurrentSemaphore{0};

        uint32_t mUpdateWidth{0};
        uint32_t mUpdateHeight{0};

        bool mVsync{false};

        void destroyResources();

    public:
        vkSwapchain& present(vk::Queue queue);
    };



} // yic

#endif //VULKAN_VKSWAPCHAIN_H
