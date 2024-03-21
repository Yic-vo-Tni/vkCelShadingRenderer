                                                         //
// Created by lenovo on 12/19/2023.
//

#include "vkSwapchain.h"

namespace yic {

    vkSwapchain& vkSwapchain::init(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Queue queue,
                                   uint32_t queueFamilyIndex, vk::SurfaceKHR surfaceKhr, vk::Format format,
                                   vk::ImageUsageFlags imageUsageFlags) {
        mDevice = device;
        mPhysicalDevice = physicalDevice;
        mSwapchain = VK_NULL_HANDLE;
        mQueue = queue;
        mQueueFamilyIndex = queueFamilyIndex;
        mSurface = surfaceKhr;
        mImageUsage = imageUsageFlags;

        auto formats{mPhysicalDevice.getSurfaceFormatsKHR(mSurface)};

        mSurfaceFormat = vk::Format::eR8G8B8A8Unorm;
        mColorSpace = formats[0].colorSpace;

        for(auto & i : formats){
            if (i.format == format){
                mSurfaceFormat = format;
                mColorSpace = i.colorSpace;
                return *this;
            }
        }

        return *this;
    }

    vkSwapchain& vkSwapchain::upDate(uint32_t width, uint32_t height, bool vsync) {
        vk::SwapchainKHR oldSwapchain = mSwapchain;

        vk::SurfaceCapabilitiesKHR capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
        std::vector<vk::PresentModeKHR> presentMode = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);

        if (capabilities.currentExtent.width == (uint32_t)-1){
            mExtent.width = width;
            mExtent.height = height;
        } else{
            mExtent = capabilities.currentExtent;
        }

        assert(mExtent.width && mExtent.height);

        vk::PresentModeKHR swPreMode = vk::PresentModeKHR::eFifo;

        if (!vsync){
            for(auto& m : presentMode) {
                if (m == vk::PresentModeKHR::eMailbox) {
                    swPreMode = vk::PresentModeKHR::eMailbox;
                }
                if (m == vk::PresentModeKHR::eImmediate) {
                    swPreMode = vk::PresentModeKHR::eImmediate;
                }
                if (swPreMode == vk::PresentModeKHR::eMailbox) {
                    break;
                }
            }
        }

        uint32_t imageCount = capabilities.minImageCount + 1;
        if((capabilities.maxImageCount > 0) && (imageCount > capabilities.maxImageCount)){
            imageCount = capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo{{},
                                              mSurface, imageCount, mSurfaceFormat, mColorSpace,
                                              mExtent, 1, mImageUsage, vk::SharingMode::eExclusive, mQueueFamilyIndex,
                                              vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                              swPreMode, vk::True, oldSwapchain};

        vkCreate([&]() { mSwapchain = mDevice.createSwapchainKHR(createInfo);}, "create swapchain");

        std::vector<vk::Image> swapchainImages{mDevice.getSwapchainImagesKHR(mSwapchain)};
        mImageCount = swapchainImages.size();
        mEntries.resize(swapchainImages.size());

        for(int i = 0; i < mImageCount; i++){
            auto& entry = mEntries[i];

            entry.image = swapchainImages[i];
            entry.imageView = vk_fn::createImageView(mDevice, entry.image, mSurfaceFormat); //createImageView(entry.image, mSurfaceFormat);
            vkTrance("create imageview " + std::to_string(i) + "successfully");

            vk::SemaphoreCreateInfo semCreateInfo{};
            entry.writtenSemaphore = mDevice.createSemaphore(semCreateInfo);
            entry.readSemaphore = mDevice.createSemaphore(semCreateInfo);
            vkInfo("create semaphore " + std::to_string(i) + "successfully");
        }

        mUpdateWidth = width;
        mUpdateHeight = height;
        mVsync = vsync;

        return *this;
    }

    bool vkSwapchain::acquireCustom(vk::Semaphore argSemaphore, int width, int height, bool *pRecreated,
                                    yic::swapChainAcquireState *pOut) {
        bool didRecreated;

        if (width != mUpdateWidth || height != mUpdateHeight){
            upDate(width, height);
            mUpdateWidth = width;
            mUpdateHeight = height;
            didRecreated = true;
        }
        if (pRecreated != nullptr){
            *pRecreated = didRecreated;
        }

        for(int i = 0; i < 2; i++){
            vk::Semaphore semaphore = argSemaphore ? argSemaphore : getActiveReadSemaphore();
            vk::ResultValue<uint32_t> rv = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE);

            if (rv.result == vk::Result::eSuccess){
                mCurrentImage = rv.value;
                if (pOut != nullptr){
                    pOut->index = getActiveImageIndex();
                    pOut->waitSem = getActiveReadSemaphore();
                    pOut->signalSem = getActiveWrittenSemaphore();
                }
                return true;
            } else if (rv.result == vk::Result::eErrorOutOfDateKHR || rv.result == vk::Result::eSuboptimalKHR){
                destroyResources();
                upDate(width, height, mVsync);
            } else { return false; };
        }

        return false;
    }

    bool vkSwapchain::acquire(bool *pRecreated, yic::swapChainAcquireState *pOut) {
        return acquireCustom(VK_NULL_HANDLE, (int )mUpdateWidth, (int )mUpdateHeight, pRecreated, pOut);
    }

    vkSwapchain &vkSwapchain::present(vk::Queue queue) {
        vk::Semaphore& written = mEntries[(mCurrentSemaphore % mImageCount)].writtenSemaphore;
        vk::PresentInfoKHR presentInfoKhr{written, mSwapchain, mCurrentImage};
        vk::Result r = queue.presentKHR(presentInfoKhr);
        return *this;
    }


    void vkSwapchain::destroyResources() {
        for(auto& it : mEntries){
            vkDestroyAll(mDevice, it.imageView, it.readSemaphore, it.writtenSemaphore);
        }
        if (mSwapchain){
            mDevice.destroy(mSwapchain);
            mSwapchain = VK_NULL_HANDLE;
        }

        mEntries.clear();
    }

} // yic