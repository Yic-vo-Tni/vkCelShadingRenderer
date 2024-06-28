//
// Created by lenovo on 12/18/2023.
//

#include "vk_init.h"

namespace yic {

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserdata ){
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    vk_init::vk_init(yic::contextCreateInfo createInfo) : info_(std::move(createInfo)){
    }

    vk_init::~vk_init() {
        vkDestroyAll(device_);
        vkDebug(instance_.destroy(debugMessenger_, nullptr, gl::dispatchLoaderDynamic_));
        vkDestroyAll(instance_);
    }

    void vk_init::oneTimeInitialization() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
    }


    vk_init &vk_init::createInstance() {
        vk::ApplicationInfo appInfo{
            info_.appName.c_str(), VK_MAKE_VERSION(1, 0, 0),
            info_.appEngine.c_str(), VK_MAKE_VERSION(1, 0, 0),
            VK_MAKE_API_VERSION(0, info_.apiMajor_, info_.apiMinor_, 0),
        };

        info_.addInstanceExtensions(co_fn::addRequiredExtensions());
        vk::InstanceCreateInfo createInfo{ {}, &appInfo};

        if(checkExtensionsSupport(info_.instanceExtensions_, info_.instanceLayers_)){
            vkDebug(createInfo.setPEnabledLayerNames(info_.instanceLayers_));
            createInfo.setPEnabledExtensionNames(info_.instanceExtensions_);
        }

        vkCreate([&](){ instance_ = vk::createInstance(createInfo); }, "create instance");

        return *this;
    }

    vk_init &vk_init::setupDebugMessenger() {
        gl::dispatchLoaderDynamic_ = vk::DispatchLoaderDynamic(instance_, vkGetInstanceProcAddr);
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{{},
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | info_.debugCreateInfo_.severityFlags,
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | info_.debugCreateInfo_.typeFlags,
                debugCallback, nullptr};

        vkDebug{vkCreate([&](){ debugMessenger_ = instance_.createDebugUtilsMessengerEXT(createInfo, nullptr, gl::dispatchLoaderDynamic_);}, "create debug message");}

        return *this;
    }

    vk_init &vk_init::createSurface() {
        if (VkSurfaceKHR tempSurface; glfwCreateWindowSurface(instance_, Window::get()->window(), nullptr, &tempSurface) != VK_SUCCESS){
            vkDebug{ vkError("failed to create surface!"); }
        } else{
            surfaceKhr_ = tempSurface;
            vkDebug{ vkInfo("create surface successfully! "); }
        }

        return *this;
    }

    vk_init &vk_init::pickPhysicalDevice() {
        for(auto physicalDevices{instance_.enumeratePhysicalDevices()}; auto& physicalDevice : physicalDevices){
            if (physicalDevice_ = physicalDevices[0], physicalDevice.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu){
                if (physicalDevice_ = physicalDevice; isDeviceSuitable(physicalDevice_)){
                    vkDebug{ vkInfo("pick the {0} successfully!", physicalDevice_.getProperties().deviceName); }
                } else { vkError("can't find the suitable physical device"); }
            }
        }
        return *this;
    }

    vk_init &vk_init::createLogicalDevice() {
        auto indices{queueFamilyIndices::findQueueFamilies(physicalDevice_, surfaceKhr_)};
        graphicsQueueFamilies = indices.graphicsFamily.value();
        mTransferQueueFamily = indices.transferFamily.value();

        std::vector<float> queuePriority{1.f};
        std::vector<uint32_t> queueFamilies{indices.graphicsFamily.value(), indices.transferFamily.value()};
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

        if (indices.graphicsFamily.value() != indices.presentFamily.value()){
            queueFamilies.push_back(indices.presentFamily.value());
        }
        for(auto& queueFamily : queueFamilies){
            queueCreateInfos.push_back({{}, queueFamily, queuePriority});
        }

        info_.features2.features.setSampleRateShading(vk::True)
                            .setWideLines(vk::True)
                            .setSamplerAnisotropy(vk::True);

        vk::DeviceCreateInfo createInfo{};
        createInfo.setQueueCreateInfos(queueCreateInfos)
                .setPEnabledExtensionNames(info_.physicalExtensions_)
                .setPNext(&info_.features2);

        vkCreate([&](){ device_ = physicalDevice_.createDevice(createInfo);}, "create logical device");
        gl::dispatchLoaderDynamic_.init(device_);

        mGraphicsQueue = device_.getQueue(graphicsQueueFamilies, 0);
        mTransferQueue = device_.getQueue(mTransferQueueFamily, 0);

        return *this;
    }


    ////////////////////////////////////////////////////////////////



    bool vk_init::isDeviceSuitable(const vk::PhysicalDevice &physicalDevice) {
        std::set<std::string> requiredExtensions(info_.physicalExtensions_.begin(), info_.physicalExtensions_.end());
        vkDebug{ vkInfo("\nthe following yellow extensions is used for device: "); }
        for(auto& extension : physicalDevice.enumerateDeviceExtensionProperties()){
            if (requiredExtensions.find(extension.extensionName) != requiredExtensions.end()) {
                vkDebug{ vkWarn(extension.extensionName); }
                requiredExtensions.erase(extension.extensionName);
            } else {
                vkDebug{ vkTrance("{0}", extension.extensionName); }
            }
        }
        return requiredExtensions.empty();
    }



    bool vk_init::checkExtensionsSupport(const std::vector<const char *> &extensions,
                                         const std::vector<const char *> &layers) {
        bool support = false;
        //extensions
        vkDebug{vkInfo("\nthe following yellow extensions is use:");}
        for(auto supportExtensions{vk::enumerateInstanceExtensionProperties()}; auto& supportExtension : supportExtensions){
            bool found = false;
            for (auto &extension: extensions) {
                if (strcmp(extension, supportExtension.extensionName) == 0) {
                    found = true; support = true;
                    vkDebug { vkWarn("\t{0} ", supportExtension.extensionName); }
                }
            } vkDebug{ if(!found) {vkTrance("\t{0}", supportExtension.extensionName);}}
        }

        //layers
        vkDebug{
            vkInfo("\nthe following yellow layers is use:");
            for(auto supportLayers{vk::enumerateInstanceLayerProperties()}; auto& supportLayer : supportLayers){
                bool found = false;
                for(auto& layer : layers){
                    if(strcmp(layer, supportLayer.layerName) == 0){
                        found = true; support = true;
                        vkDebug{ vkWarn("\t{0} ", supportLayer.layerName);}
                    }
                } if (!found){vkTrance("\t{0}", supportLayer.layerName);}
            }
        }

        //if
        if(!support){
            vkDebug{ vkError("some extensions or layers must be support!"); }
            return false;
        }

        return true;
    }



} // yic