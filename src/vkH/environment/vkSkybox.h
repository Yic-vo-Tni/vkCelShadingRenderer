//
// Created by lenovo on 2/3/2024.
//

#ifndef VKMMD_VKSKYBOX_H
#define VKMMD_VKSKYBOX_H

#include "vkInit/vk_init.h"

#include "niLou/vkDescriptor.h"
#include "niLou/vkPipeline.h"
#include "niLou/vkSwapchain.h"
#include "niLou/vkCamera.h"

#include "ke_q/file_operation.h"

#include "centralizedTaskSystem/vkCommon.h"

namespace yic {

    class vkSkybox : public nonCopyable{
        enum skyPipeTypes{
            eSkybox,
        };

        struct cameraVec{
            glm::mat4 vpMatrix;
        };
    public:
        vkSkybox(vk_init* vkInit, vkSwapchain& swapchain);

        vkSkybox& renderSkybox();
        vkSkybox& updateEveryFrame(){
            updateCameraVec();
            return recreate();
        }

        vk::CommandBuffer& getActiveCmdBuf() {return mCommandBuffers[mSwapchain.getActiveImageIndex()];};

    private:
        vkSkybox& createSkyRenderPass();
        vkSkybox& createSkyboxPipeline();
        vkSkybox& createSkyFrameBuffer();
        vkSkybox& taskAssignment();
        vkSkybox& updateSkyDescriptor();

        vkSkybox& updateCameraVec();
        vkSkybox& recreate();
        vk::CommandBuffer createTempCmdBuf();
        void submitTempCmdBuf(vk::CommandBuffer& cmd);

        vkSkybox& prepareSkyboxEveryThing();

    private:
        vk::Device mDevice;
        vk::Queue mGraphicsQueue;
        vk::Extent2D mExtent;
        vk::CommandPool mCommandPool;
        uint32_t mGraphicsIndex;
        std::vector<vk::CommandBuffer> mCommandBuffers;

        cameraVec mCameraVec{};
        vkSwapchain& mSwapchain;

        std::unordered_map<skyPipeTypes, vk::RenderPass> mSkyboxRenderPass;
        std::unordered_map<skyPipeTypes, vk::PipelineLayout> mSkyboxPipeLayout;
        std::unordered_map<skyPipeTypes, vk::Pipeline> mSkyboxPipeline;
        std::unordered_map<skyPipeTypes, std::vector<vk::Framebuffer>> mSkyFrameBuffers;
        std::unordered_map<skyPipeTypes, vkDescriptor> mSkyboxDescriptor;

        genericBufferManagerUptr mCameraVecBuf{};
        genericSkyboxManagerSptr mSkybox{};
    };

} // yic

#endif //VKMMD_VKSKYBOX_H
