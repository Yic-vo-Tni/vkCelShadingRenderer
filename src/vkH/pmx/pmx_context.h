//
// Created by lenovo on 1/13/2024.
//

#ifndef VKMMD_PMX_CONTEXT_H
#define VKMMD_PMX_CONTEXT_H

#include "pmx/pmx_header.h"
#include "editor/modelTransformManager.h"

namespace vkPmx {

    using namespace yic;

    class pmx_context {
    private:
        struct pmxModelResource{
            // vertex buf
            vk::IndexType mIndexType{};
            uint32_t mMmdVertUniformBufOffset{};

            allocManager::bufAccelAddressUptr mIndexBuffer{};
            allocManager::bufAccelAddressUptr mVertexBuffer{};

            // uniform buf
            std::vector<uint32_t> mMmdFragUniformBufOffset{};
            allocManager::bufUptr mUniformBuffer{};
        };

    public:
        pmx_context(const std::vector<Input>& inputModels, vk_init* vkInit, uint32_t imageCount, const vk::RenderPass& renderPass, const vk::CommandPool& commandPool);
        [[nodiscard]] vk::CommandBuffer getCmdBuf(uint32_t imageIndex) const { return mCmdBuffers[imageIndex]; }
    private:
        pmx_context& createPipeline();
        pmx_context& createVertexBuffer();
        pmx_context& createDescriptorSet();
        pmx_context& createCommandBuffer();
        vk::CommandBuffer createTempCmdBuf();
        void submitTempCmdBuf(vk::CommandBuffer& cmd);

        pmx_context& loadPmxInputModels(const std::vector<Input>& inputModels);

    public:
        pmx_context& createMaterialsTex();
        pmx_context& prepareEveryThing();

        pmx_context& updateEveryFrame(const glm::mat4& view, const glm::mat4& proj);
        pmx_context& updateAnimation();
        pmx_context& update();
        pmx_context& draw(const vk::CommandBuffer& cmd);

    public:
        [[nodiscard]] inline auto& getPmxModel() const { return mPmxModel;}

    private:
        std::shared_ptr<saba::MMDModel> mMmdModel;
        std::unique_ptr<saba::VMDAnimation> mVmdAnimation;

    private:
        yic::vk_init* mvkInit;

        uint32_t mImageCount{};
        vk::Device mDevice;
        vk::PhysicalDevice mPhysicalDevice;
        vk::RenderPass mRenderPass;
        vk::CommandPool mCommandPool;

        vk::Pipeline mPipeline;
        vk::PipelineLayout mPipelineLayout;
        vkDescriptor mMmdDescriptor;

    private:
        std::string mResDir;
        std::string mShaderDir;
        std::string mMmdDir;

        double mSaveTime{};
        float mElapsed{0.f}, mAnimTime{0.f};
        glm::mat4 mView{}, mProj{};
        glm::vec3 mLightColor{1, 1, 1};
        glm::vec3 mLightDir{-0.5f, -1.0f, -0.5f};
        size_t mMaterialCount{0};
    private:
        pmxModelResource mModelResource{};
        pmxModel mPmxModel{};
        std::vector<vk::CommandBuffer> mCmdBuffers;

        std::vector<genericTexManagerSptr> mMmdMaterials;

        glm::vec3 min{FLT_MAX}, max{-FLT_MAX};
    };

} // vkPmx

#endif //VKMMD_PMX_CONTEXT_H
