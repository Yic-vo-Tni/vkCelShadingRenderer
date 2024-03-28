//
// Created by lenovo on 3/28/2024.
//

#include "modelTransformManager.h"

namespace yic {

    void modelTransformManager::addPmxModel(const std::string& pmxPath) {
        mInputModels.emplace_back(pmxPath);
    }

    void modelTransformManager::addPmxVMDAnim(const std::string& vmdPath) {
        std::vector<std::string> vmd{vmdPath};
        mInputModels[0].m_vmdPaths = vmd;
    }

} // yic