//
// Created by lenovo on 1/7/2024.
//

#ifndef VKMMD_VKPMXMODEL_H
#define VKMMD_VKPMXMODEL_H

#include <Saba/Model/MMD/PMDModel.h>
#include <Saba/Model/MMD/PMXModel.h>
#include <Saba/Model/MMD/VMDFile.h>
#include <Saba/Model/MMD/VMDAnimation.h>
#include <Saba/Model/MMD/VMDCameraAnimation.h>

namespace pmx {

    struct pmxMode{
        std::shared_ptr<saba::MMDModel> mMmdModel;
        std::unique_ptr<saba::VMDAnimation> mVmdAnim;

        struct material{

        };
    };

} // pmx

#endif //VKMMD_VKPMXMODEL_H
