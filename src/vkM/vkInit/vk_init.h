//
// Created by lenovo on 12/18/2023.
//

#ifndef VULKAN_VK_INIT_H
#define VULKAN_VK_INIT_H

#include "yic_pch.h"

#include "vk_window.h"

#include "miku/context_vkinit.h"
#include "miku/rhi_co.h"

namespace yic {

    class vk_init : public nonCopyable{
        explicit vk_init(contextCreateInfo  createInfo = {});
        ~vk_init();
        friend Singleton<vk_init>;

        contextCreateInfo info_;
    public:
        vkGet auto get = [](const contextCreateInfo& Info = {}){ return Singleton<vk_init>::get(Info); };

        void oneTimeInitialization();
        vk_init& createInstance();
        vk_init& setupDebugMessenger();
        vk_init& createSurface();
        vk_init& pickPhysicalDevice();
        vk_init& createLogicalDevice();

        [[nodiscard]] inline auto& getInstance() const { return instance_;}
        [[nodiscard]] auto& physicalDevice() const { return physicalDevice_;}
        [[nodiscard]] auto& device() const { return device_;}
        [[nodiscard]] auto& surfaceKhr() const { return surfaceKhr_;}

        [[nodiscard]] auto& graphicsQueue() const { return mGraphicsQueue;}
        [[nodiscard]] auto& getGraphicsFamilyIndices() const { return graphicsQueueFamilies; }

        [[nodiscard]] inline auto& getTransferQueue() const { return mTransferQueue;}
        [[nodiscard]] inline auto& getTransferQueueFamily() const { return mTransferQueueFamily;}

    private:
        vk::Instance instance_{VK_NULL_HANDLE};

        vk::DispatchLoaderDynamic dispatchLoaderDynamic_;
        vk::DebugUtilsMessengerEXT debugMessenger_{VK_NULL_HANDLE};

        vk::SurfaceKHR surfaceKhr_{VK_NULL_HANDLE};

        vk::PhysicalDevice physicalDevice_{VK_NULL_HANDLE};
        vk::Device device_{VK_NULL_HANDLE};
        vk::Queue mGraphicsQueue{VK_NULL_HANDLE};
        vk::Queue mTransferQueue{VK_NULL_HANDLE};

        uint32_t graphicsQueueFamilies{UINT32_MAX};
        uint32_t mTransferQueueFamily{UINT32_MAX};

        bool checkExtensionsSupport(const std::vector<const char*>& extensions, const std::vector<const char*>& layers);
        bool isDeviceSuitable(const vk::PhysicalDevice& physicalDevice);
    };

} // yic

#endif //VULKAN_VK_INIT_H
