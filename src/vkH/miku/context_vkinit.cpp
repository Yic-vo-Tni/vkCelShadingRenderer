//
// Created by lenovo on 12/18/2023.
//
#include "context_vkinit.h"

namespace yic{

    contextCreateInfo::contextCreateInfo() = default;

    contextCreateInfo &contextCreateInfo::setVersion(uint32_t major, uint32_t minor) {
        assert(apiMajor_ == 1 && apiMinor_ >= 1);
        apiMajor_ = major;
        apiMinor_ = minor;
        return *this;
    }

///////////////////// instance

    contextCreateInfo &contextCreateInfo::addUtilsMessageFlags(const yic::contextCreateInfo::debugCreateInfo& debugMessage) {
        debugCreateInfo_.severityFlags |= debugMessage.severityFlags;
        debugCreateInfo_.typeFlags |= debugMessage.typeFlags;

        return *this;
    }









































    contextCreateInfo &contextCreateInfo::addInstanceLayer(const char *layer, bool optimal) {
        if (optimal){ instanceLayers_.push_back(layer); }
        return *this;
    }
    contextCreateInfo &contextCreateInfo::addInstanceExtension(const char *extension, bool optimal) {
        if (optimal){ instanceExtensions_.push_back(extension); }
        return *this;
    }

    contextCreateInfo &contextCreateInfo::addInstanceLayersList(const std::initializer_list<layerInfo> layers) {
        for(const auto& layer : layers){
            if (layer.optimal){
                instanceLayers_.push_back(layer.layer);
            }
        }
        return *this;
    }
    contextCreateInfo &contextCreateInfo::addInstanceExtensionsList(const std::initializer_list<extensionInfo> extensions) {
        for(const auto& extension : extensions){
            if (extension.optimal){
                instanceExtensions_.push_back(extension.extension);
            }
        }
        return *this;
    }


    //////////////////////////////////////////

    std::vector<const char*> co_fn::addRequiredExtensions() {
        uint32_t glfwExtensionsCount{};
        auto glfwExtensions{glfwGetRequiredInstanceExtensions(&glfwExtensionsCount)};

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

        vkDebug{
            std::cout << "extensions is required: \n";
            for(auto& extension : extensions){
                vkWarn("{0}", extension);
            }
        }

        return extensions;
    }












}