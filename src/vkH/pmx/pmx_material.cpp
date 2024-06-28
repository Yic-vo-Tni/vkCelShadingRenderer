//
// Created by lenovo on 5/2/2024.
//

#include "pmx_material.h"

namespace vkPmx {

    pmx_material::pmx_material(const std::string &path, vk::CommandBuffer& cmd) {
        mTexture = std::make_shared<vkImage>(path);
        mTexture->configureImageForRender(cmd);

        mUniformEffectBuf = allocManager::build::bufSptr(sizeof (MMDFragEffect), vk::BufferUsageFlagBits::eUniformBuffer);

        auto npos = std::string::npos;
        if (path.find("颜") != npos){
            name = "face";
        } else if (path.find("髮") != npos){
            name = "hair";
        } else if (path.find("体") != npos){
            name = "body";
        } else if (path.find("肌1") != npos){
            name = "skin_1";
        } else if(path.find("肌2") != npos){
            name = "skin_2";
        }
    }


} // yic