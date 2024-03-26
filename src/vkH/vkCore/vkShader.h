//
// Created by lenovo on 3/25/2024.
//

#ifndef VKMMD_VKSHADER_H
#define VKMMD_VKSHADER_H

#include "log/Log.h"

namespace vkHelp{

    [[nodiscard]] inline vk::ShaderModule createShaderModule(vk::Device device, const uint32_t* binaryCode, size_t sizeInBytes){
        vk::ShaderModuleCreateInfo createInfo{{}, sizeInBytes, binaryCode};

        vk::ShaderModule shaderModule;
        vkCreate([&]() { shaderModule = device.createShaderModule(createInfo);}, "create shader Module");
        return shaderModule;
    }

    [[nodiscard]] inline vk::ShaderModule createShaderModule(vk::Device device, const std::string & code){
        return createShaderModule(device, (const uint32_t*)code.data(), code.size());
    }

}

#endif //VKMMD_VKSHADER_H
