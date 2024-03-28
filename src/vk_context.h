//
// Created by lenovo on 12/19/2023.
//

#ifndef VULKAN_VK_CONTEXT_H
#define VULKAN_VK_CONTEXT_H

#include "miku/vk_base.h"
#include "ke_q/vk_allocator.h"
#include "niLou/vkImage.h"
#include "editor/vkImgui.h"

#include "scene/vkMeshes.h"
#include "scene/vkScene.h"

#include "vkInit/vk_init.h"

#include "pmx/pmx_context.h"
#include "environment/vkSkybox.h"

namespace yic {

    class vk_context : public vk_base {
    public: // init
        vk_context();
        ~vk_context();
        vk_context& init();
        // render
        vk_context& updateEveryFrame();
        vk_context& orRender();
        vk_context& prepareEveryContext();

        vk_context& rasterization(const vk::CommandBuffer& cmd);

        // front cmd
        [[nodiscard]] std::vector<vk::CommandBuffer> getActiveFrontCmdBuf() const{
            std::vector<vk::CommandBuffer> cmdBuffers;
            cmdBuffers.push_back(mSkyboxContext->getActiveCmdBuf());

            return cmdBuffers;
        }

        [[nodiscard]] inline auto& getPmxModel() const { return mPmxModel;}

    private: // pmx
        vk_context& prePmxContext();
        // skybox
        vk_context& preSkyContext();

    private:
        std::unique_ptr<vkPmx::pmx_context> mPmxContext{}, mBackupContext;
        std::unique_ptr<vkSkybox> mSkyboxContext{};

    private: /// transfer buf
        vk::CommandPool mTransferPool{};

        vkPmx::pmxModel mPmxModel;
        std::thread mPmxThread;
    };

} // yic

#endif //VULKAN_VK_CONTEXT_H













// setup
//        vk_context& createGraphicsPipeline() override;
//        vk_context& updateDescriptorSet() override;
//        vk_context& prepare();


//    private: /// Rhi
//        vkDescriptor mDescriptor{};
//        vk::PipelineLayout mPipelineLayout{VK_NULL_HANDLE};
//        vk::Pipeline mGraphicsPipeline{VK_NULL_HANDLE};
//
//    private: /// Prep
//        std::unique_ptr<vkScene> mScene = std::make_unique<vkScene>();
//
//    private: /// Tex
//        genericTexManagerUptr mImage;
//
//    private: /// Vert
//        std::vector<vk::Buffer> mBindBuffers;
//        std::vector<vk::DeviceSize> mOffsets;
//        std::unique_ptr<genericBufferManager> mVertexBuffer{};
//        std::unique_ptr<vkMeshes> mMeshes = std::make_unique<vkMeshes>();


//    private: /// MVP
//        // M
//        std::unique_ptr<genericBufferManager> mModelBuffer{};
//        // VP
//        std::unique_ptr<genericBufferManager> mCameraBuffer{};