//
// Created by lenovo on 4/24/2024.
//

#include "lightManager.h"

namespace yic {

    lightManager::lightManager() = default;

    void lightManager::addDirectionalLight() {
        auto dirLight = std::make_shared<directionalLight>();
        mDirectionLight = dirLight;
        mLight.emplace_back(dirLight);
    }

} // yic