//
// Created by lenovo on 3/28/2024.
//

#ifndef VKMMD_MODELTRANSFORMMANAGER_H
#define VKMMD_MODELTRANSFORMMANAGER_H

#include "pmx/pmx_header.h"

namespace yic {

    class modelTransformManager {
    public:
        vkGet auto get = []() { return Singleton<modelTransformManager>::get();};

        void setPosition(const glm::vec3& pos){ mPos = pos; }
        void setAnimPlay() { mIsAnimationPlaying = !mIsAnimationPlaying; }

        void addPmxModel(const std::string& pmxPath);
        void addPmxVMDAnim(const std::string& vmdPath);

        void setRenderPmxModel() { mIsRenderPmxModel = !mIsRenderPmxModel; }
        void setLoadPmxModel() { mIsLoadPmxModel = !mIsLoadPmxModel; }
        void setLoadVMDAnim() { mIsLoadVmdAnim = !mIsLoadVmdAnim; }

        [[nodiscard]] inline auto& getPos() const { return mPos;}
        [[nodiscard]] inline auto getModelMatrix() const {
            glm::mat4 mModelMatrix = glm::translate(glm::mat4 (1.f), mPos);
            return  mModelMatrix;
        }
        [[nodiscard]] inline auto& playAnimVMD() const { return mIsAnimationPlaying;}

        [[nodiscard]] inline auto& getInputModels() const { return mInputModels;}
        [[nodiscard]] inline auto& isRenderPmxModel() const { return mIsRenderPmxModel;}
        [[nodiscard]] inline auto& isLoadPmxModel() const { return mIsLoadPmxModel;}
        [[nodiscard]] inline auto& isLoadVMDAnim() const { return mIsLoadVmdAnim;}

    private:
        glm::vec3 mPos{0.f};
        bool mIsAnimationPlaying = false;

        std::vector<vkPmx::Input> mInputModels{};
        bool mIsRenderPmxModel{false}, mIsLoadPmxModel{false}, mIsLoadVmdAnim{false};
    };


} // yic

#endif //VKMMD_MODELTRANSFORMMANAGER_H
