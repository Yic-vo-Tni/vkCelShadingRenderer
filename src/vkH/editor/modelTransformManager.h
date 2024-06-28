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
        void setScale(const float& scale){ mScale = scale;}
        void setAnimPlay() { mIsAnimationPlaying = !mIsAnimationPlaying; }
        void setUpdateAnim(bool update) { mUpdateAnim = update; }

        void addPmxModel(const std::string& pmxPath);
        void addPmxVMDAnim(const std::string& vmdPath);

        void setInputModels(const vkPmx::Input& input) {std::lock_guard<std::mutex> guard(mVmdMutex); mInputModels.emplace_back(input);}
        void setRenderPmxModel() { mIsRenderPmxModel = !mIsRenderPmxModel; }
        void setLoadPmxModel() { std::lock_guard<std::mutex> guard(mMutex); mIsLoadPmxModel = !mIsLoadPmxModel; }
        void setLoadVMDAnim() { mIsLoadVmdAnim = true; }

        [[nodiscard]] inline auto& getPos() const { return mPos;}
        [[nodiscard]] inline auto getModelMatrix() const {
            glm::mat4 mModelMatrix = glm::translate(glm::mat4 (1.f), mPos);
            return  mModelMatrix;
        }
        [[nodiscard]] inline auto& getScale() const { return mScale;}
        [[nodiscard]] inline auto getScaleMatrix() const {
            glm::mat4 mScaleMatrix = glm::scale(glm::mat4 (1.f), glm::vec3 (mScale));
            //vkWarn(mScale);
            return  mScaleMatrix;
        }
        [[nodiscard]] inline auto& getVmdAnim() const { return mVMDAnim;}
        [[nodiscard]] inline auto& getVmdAnim()  { return mVMDAnim;}
        [[nodiscard]] inline auto& playAnimVMD() const { return mIsAnimationPlaying;}

        [[nodiscard]] inline auto& getInputModels()  { std::lock_guard<std::mutex> guard(mVmdMutex); return mInputModels;}
        [[nodiscard]] inline auto& isRenderPmxModel() const { return mIsRenderPmxModel;}
        [[nodiscard]] inline auto& isLoadPmxModel()  {std::lock_guard<std::mutex> guard(mMutex); return mIsLoadPmxModel;}
        [[nodiscard]] inline auto& isLoadVMDAnim() const { return mIsLoadVmdAnim;}
        [[nodiscard]] inline auto& getUpdateAnim() const { return mUpdateAnim;}

    private:
        glm::vec3 mPos{0.f};
        float mScale{1.f};
        bool mIsAnimationPlaying = false;
        bool mUpdateAnim = false;
        float mVMDAnim{};

        std::vector<vkPmx::Input> mInputModels{};
        bool mIsRenderPmxModel{false}, mIsLoadPmxModel{false}, mIsLoadVmdAnim{false};
        std::mutex mMutex, mVmdMutex;
    };


} // yic

#endif //VKMMD_MODELTRANSFORMMANAGER_H
