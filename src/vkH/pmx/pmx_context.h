//
// Created by lenovo on 1/13/2024.
//

#ifndef VKMMD_PMX_CONTEXT_H
#define VKMMD_PMX_CONTEXT_H

#include "pmx/pmx_header.h"
#include "pmx/pmx_material.h"
#include "editor/modelTransformManager.h"
#include "configurationRender/vkRenderTaskQueue.h"
#include "editor/vkImguiTaskQueue.h"
#include "ffd.h"
#include "postProcessing/directionShadowMapManager.h"

namespace vkPmx {

    using namespace yic;

    struct MeshState{
        glm::vec3 transform{0.f};
        glm::vec3 scale{1.f};
        glm::vec3 center{1.f};
        saba::MMDSubMesh subMesh{};
        allocManager::bufUptr mUniformBuf{};

//        MMDFragEffect fragEffect{0.6f};
//        allocManager::bufSptr mUniformEffectBuf{};

        std::shared_ptr<ffd> ffd{};
        bool drawAABB{false};
        float distance{0.6f};
        glm::vec3 min{FLT_MAX}, max{-FLT_MAX};
        int imguiTransform{-1};
        MMDFragmentShaderUB materialInfo{};
        bool mmdTextureEmpty{false};
        bool alpha{true};
        float color[4] = {0.0f, 0.0f, 0.0f, 0.f};
    };

    class pmx_context {
    private:
        struct pmxModelResource{
            // vertex buf
            vk::IndexType mIndexType{};
            uint32_t mMmdVertUniformBufOffset{};
            uint32_t mGroundShadowBufOffset{};

            allocManager::bufAccelAddressUptr mIndexBuffer{};
            allocManager::bufAccelAddressUptr mVertexBuffer{};
        };

    public:
        pmx_context(const std::vector<Input>& inputModels, vk_init* vkInit, uint32_t imageCount, const vk::RenderPass& renderPass);
        ~pmx_context();
        [[nodiscard]] vk::CommandBuffer getCmdBuf(uint32_t imageIndex) const { return mCmdBuffers[imageIndex]; }
    private:
        pmx_context& createPipeline();
        pmx_context& createShadowPipeline();
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

        pmx_context& drawDirectLightShadowMap(const vk::CommandBuffer& cmd);

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
        vkDescriptor mImguiTexDescriptor;

        vk::Pipeline mGroundShadowPipeline;
        vk::PipelineLayout mGroundShadowPipelineLayout;
        vkDescriptor mGroundShadowDescriptor;

    private:
        std::string mResDir;
        std::string mShaderDir;
        std::string mMmdDir;

        double mSaveTime{};
        float mElapsed{0.f}; float mAnimTime{0.f};
        glm::mat4 mView{}, mProj{};
        glm::vec3 mLightColor{1, 0.8, 0.8};
        glm::vec3 mLightDir{0.5f, 1.0f, 0.5f};
        size_t mMaterialCount{0};
    private:
        pmxModelResource mModelResource{};
        pmxModel mPmxModel{};
        std::vector<vk::CommandBuffer> mCmdBuffers;
        std::vector<MeshState> mSubMesh{};
      //  FragEffect mFragEffect{25.f, 0.2f, 2.3f};
        MMDFragEffect mMMDFragEff{0.6f,
                                  10.f,
                                  0.2f,
                                  2.3f,
                                  0.f,
                                  1.f,
                                  0.5f,0,
                                  3, 0.3f, 0.35f, 0.2f, 0.03f,
                                  0.f, 0.f, 0.f, 0.f,
                                  0.2f, 0.2f, 0.001f, 1.3f
        };

        //
        std::vector<std::shared_ptr<pmx_material>> mMaterials;
        std::unordered_map<std::string, std::shared_ptr<pmx_material>> mSptrMaterials;
        std::vector<std::function<void()>> mUpdateDescriptorSet{};

        glm::vec3 min{FLT_MAX}, max{-FLT_MAX};
        glm::vec3 mCenter{};
        int mSliderAnim{-1};
        bool mPlayAnim{false}, mLoadAnim{false};
        bool mUniformScale{true};

        std::shared_ptr<ffd> mRayIntersect{};
        float minDistance = std::numeric_limits<float>::max();
        int nearestIndex = -1;

        bool mMove_x{false}, mMove_y{false}, mMove_z{false};

        allocManager::bufSptr mDirectionLightSpaceMatrixBuf{};
        directionShadowMapManager mDirShadowMap{};

        // blush
        std::shared_ptr<vkImage> mFaceBlush;
        // ramp
        // std::shared_ptr<vkImage> mRamp;

        // test
        float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        float hdr = {0.f};
        bool hdrJudge = {false};
    };

} // vkPmx

#endif //VKMMD_PMX_CONTEXT_H
