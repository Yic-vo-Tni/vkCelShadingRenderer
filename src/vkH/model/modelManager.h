//
// Created by lenovo on 4/22/2024.
//

#ifndef VKMMD_MODELMANAGER_H
#define VKMMD_MODELMANAGER_H

#include "load.h"

#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"

#include "vkInit/vk_init.h"

#include "editor/vkImguiTaskQueue.h"

#include "postProcessing/shadowMap.h"
#include "postProcessing/directionShadowMapManager.h"
#include "illuminant/lightManager.h"

#include "niLou/vkCamera.h"

namespace yic {

    class modelManager {
        struct MVP{
            glm::mat4 vpMatrix;
        };
    public:
        vkGet auto get = [](){ return Singleton<modelManager>::get();};
        modelManager() = default;
        ~modelManager() = default;

        void init(vk_init* vkInit,  uint32_t imageCount, const vk::RenderPass& renderPass);

        void draw(const vk::CommandBuffer& cmd);
        void drawShadowMap(const vk::CommandBuffer& cmd);
        bool updateEveryFrame(glm::mat4 vp);

        void updatePath(const std::string& path){
            mPath = path;
        };

    private:
        void loadModel(const std::string& path);
        bool createCmdBuf();
        bool createDefaultPipeline();

    private:
        // import
        vk::Device mDevice{};
        vk::RenderPass mRenderPass{};
        vk::Queue mGraphicsQueue{};
        uint32_t mImageCount{UINT32_MAX};

        //
        vk::CommandPool mCmdPool{};
        std::vector<vk::CommandBuffer> mCmdBufs{};
        vk::PipelineLayout mDefaultPipeLayout{};
        vk::Pipeline mDefaultPipeline{};
        vkDescriptor mDescriptor{};

        MVP mMvp{};
        //
        std::string mPath{};
        std::vector<std::shared_ptr<load>> mLoadModels;

        directionShadowMapManager mDirShadowMap;
    };

} // yic

#endif //VKMMD_MODELMANAGER_H
