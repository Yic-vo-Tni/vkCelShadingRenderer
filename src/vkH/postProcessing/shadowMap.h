//
// Created by lenovo on 4/24/2024.
//

#ifndef VKMMD_SHADOWMAP_H
#define VKMMD_SHADOWMAP_H

#include <ke_q/file_operation.h>

#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"
#include "niLou/vkImage.h"

#include "miku/vk_fn.h"

#include "editor/vkImguiTaskQueue.h"

#include "model/modelData.h"
#include "illuminant/lightManager.h"

namespace yic {

    struct shadowMapConfiguration{
        // base
        float texelSize{};
        float bias{};
        float shadow{0.8};

        // pcf
        bool enablePcf{false};
        int pcf_para{1};
        int pcf{ static_cast<int>(std::pow((pcf_para * 2 + 1), 2))};

        // light
        float posCameraX{};
        float posCameraY{};
        float posCameraZ{};
        float colorLightX{};
        float colorLightY{};
        float colorLightZ{};

//        glm::vec3 posCamera{0.f, 0.f, 0.f};
//        float pad_0{};
//        glm::vec3 colorLight{1.1f, 0.9f, 0.9f};
//        float pad_1{};
    };

    class shadowMap {
    public:
        shadowMap();

        vkGet auto get = [](){ return Singleton<shadowMap>::get();};

        bool init(vk::Device device, vk::PhysicalDevice physicalDevice);
        vk::CommandBuffer& renderShadowMapBegin();
        bool renderShadowMapEnd();

        void updateEveryFrame(){
            mConfiguration.posCameraX = vkCamera::get()->position.x;
            mConfiguration.posCameraY = vkCamera::get()->position.y;
            mConfiguration.posCameraZ = vkCamera::get()->position.z;
            mConfiguration.colorLightX = lightManager::getDirectionLight()->getLightColor().x;
            mConfiguration.colorLightY = lightManager::getDirectionLight()->getLightColor().y;
            mConfiguration.colorLightZ = lightManager::getDirectionLight()->getLightColor().z;
            //mConfiguration.posCamera = lightManager::getDirectionLight()->getLightDirect();

            //vkWarn("{0}, {1}, {2}",mConfiguration.colorLight.x, mConfiguration.colorLight.y, mConfiguration.colorLight.z);
            mConfigBuf->updateBuffer(mConfiguration);
        }

        [[nodiscard]] inline auto& getShadowMapView() const { return mShadowMapView;}
        [[nodiscard]] inline auto& getShadowMapRenderPass() const { return mShadowRenderPass;}
        [[nodiscard]] inline auto& getShadowMapPipeline() const { return mShadowMapPipeline;}
        [[nodiscard]] inline auto& getShadowMapPipelineLayout() const { return mShadowMapPipelineLayout;}
        [[nodiscard]] inline auto& getShadowMapCmd() const { return mShadowMapCmd;}
        [[nodiscard]] inline auto& getShadowMapDirectionLightSet() const { return mShadowMapDescriptor;}

        [[nodiscard]] inline auto& getConfigurationBuf() const { return mConfigBuf;}

    private:
        bool createShadowMap();
        bool createShadowMapView();
        bool createShadowMapRenderPass();
        bool createShadowMapFrameBuffer();
        bool createShadowMapPipeline();
        bool createShadowMapCmd();

        bool getDepthFormat();

        bool updateImguiShadowMapSet();
        bool updateDirectionLightShadowMapSet();

    private:
        vk::Device mDevice;
        vk::PhysicalDevice mPhysicalDevice;

        // create
        vk::Format mShadowMapFormat{};
        vk::Extent2D mExtent{2048, 2048};
        vk::Image mShadowMap{};
        vk::ImageView mShadowMapView{};
        vk::DeviceMemory mShadowMapMemory{};

        vk::Image mColorShadowMap{};
        vk::ImageView mColorShadowMapView{};
        vk::DeviceMemory mColorShadowMapMemory{};

        vk::RenderPass mShadowRenderPass{};
        vk::Pipeline mShadowMapPipeline{};
        vk::PipelineLayout mShadowMapPipelineLayout{};

        vk::Framebuffer mShadowMapFrameBuf{};
        vk::CommandPool mShadowMapCmdPool{};
        vk::CommandBuffer mShadowMapCmd{};

        vkDescriptor mShadowMapDescriptor{};
        vkDescriptor mImguiShadowMapDescriptor{};
        vk::Sampler mShadowSampler{};

        shadowMapConfiguration mConfiguration{};
        allocManager::bufSptr mConfigBuf{};
    };

} // yic

#endif //VKMMD_SHADOWMAP_H
