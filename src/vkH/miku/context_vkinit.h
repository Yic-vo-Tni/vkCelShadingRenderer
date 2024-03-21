//
// Created by lenovo on 12/18/2023.
//

#ifndef VULKAN_CONTEXT_VKINIT_H
#define VULKAN_CONTEXT_VKINIT_H

#include "log/Log.h"
#include "yic_pch.h"

namespace yic{

    struct contextCreateInfo{
        contextCreateInfo();
        vkGet auto get = [](){ return Singleton<contextCreateInfo>::get(); };

        contextCreateInfo& setVersion(uint32_t major = 1, uint32_t minor = 1);

        template<typename... Args>
        contextCreateInfo& addInstanceLayers(const char* layer, Args...args){
            instanceLayers_.push_back(layer);
            (instanceLayers_.push_back(args), ...);
            return *this;
        }
        template<typename... Args>
        contextCreateInfo& addInstanceExtensions(const char* extensions, Args...args){
            instanceExtensions_.push_back(extensions);
            (instanceExtensions_.push_back(args), ...);
            return *this;
        }
        contextCreateInfo& addInstanceExtensions(const std::vector<const char*>& extensions){
            for(auto& extension : extensions){
                instanceExtensions_.push_back(extension);
            }
            return *this;
        }

        template<typename... Args>
        contextCreateInfo& addPhysicalDeviceExtensions(const char* extensions, Args...args){
            physicalExtensions_.push_back(extensions);
            (physicalExtensions_.push_back(args), ...);
            return *this;
        }

        template<class Feature>
        contextCreateInfo& addPhysicalDeviceExtensions(const  char* extension, Feature* feature){
            addPhysicalDeviceExtensions(extension);
            if (feature){
                addFeatureToChain(feature);
            }
            return *this;
        }

    public:
        std::string appEngine{"NiLou"}, appName{"KaWaYi"};

        uint32_t apiMajor_{1}, apiMinor_{3};
        std::vector<const char*> instanceExtensions_{};
        std::vector<const char*> instanceLayers_{};

        vk::PhysicalDeviceFeatures2 features2{};

    private:
        template<class featureType>
        void addFeatureToChain(featureType& feature){
            feature->pNext = features2.pNext;
            features2.pNext = &feature;
        }
        //////// instance

    public:
        struct debugCreateInfo{
            vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{};
            vk::DebugUtilsMessageTypeFlagsEXT typeFlags{};
        } debugCreateInfo_{};

        contextCreateInfo& addUtilsMessageFlags(const debugCreateInfo& debugMessage);

        ////////////// physical device
        std::vector<const char*> physicalExtensions_{};

        ///////////// logical device










































    private:
        struct extensionInfo{
            const char* extension;
            bool optimal;
            extensionInfo(const char* ex, bool op = true) : extension(ex), optimal(op) {}
        };
        struct layerInfo{
            const char* layer;
            bool optimal;
            layerInfo(const char* lay, bool op = true) : layer(lay), optimal(op) {}
        };

        contextCreateInfo& addInstanceLayer(const char* layer, bool optimal = true);
        contextCreateInfo& addInstanceExtension(const char* extension, bool optimal = true);

        contextCreateInfo& addInstanceLayersList(const std::initializer_list<layerInfo> layers);
        contextCreateInfo& addInstanceExtensionsList(const std::initializer_list<extensionInfo> extensions);

    };


    ////////////////////////////////////

    struct co_fn{
        static std::vector<const char*> addRequiredExtensions();
    };

}

#endif //VULKAN_CONTEXT_VKINIT_H
