//
// Created by lenovo on 4/24/2024.
//

#ifndef VKMMD_LIGHTBASE_H
#define VKMMD_LIGHTBASE_H

#include "ke_q/vk_allocator.h"

#include "editor/vkImguiTaskQueue.h"

namespace yic {

    enum lightType{
        eDirectionLight, ePointLight, eSpotlight
    };

    struct lightState{
        glm::vec3 color;
        float intensity;
        glm::vec3 position;
        glm::vec3 direction;
        float constantAttenuation;
        float linearAttenuation;
        float quadraticAttenuation;
        float cutoffAngle;
        float falloff;

        lightType type;
        bool enabled;
    };

    struct lightCamera{
//        glm::mat4 view{};
//        glm::mat4 proj{};
        glm::mat4 vp{};
    };

    struct LightVpMatrix{
        glm::mat4 vp{};
    };

    class lightBase {
    public:
        lightBase() : mLightState({1.f, 1.f, 1.f},
                                  1.f, //亮度水平
                                  {0.f, 0.f, 0.f},
                                  {0.f, 0.f, -1.f},
                                  1.f, //不随距离变化而衰减
                                  0.f, //无线性衰减
                                  0.01f, //轻微的距离衰减
                                  45.f, //初始聚光角度
                                  1.f, //边缘开始显著衰减
                                  ePointLight,
                                  true) {};
        virtual ~lightBase() = default;

        virtual void update() = 0;

        [[nodiscard]] inline auto& getVpMatrixBuf() const { return mVpUniformBuf;}
        [[nodiscard]] inline glm::mat4 getLightSpaceMatrix() const { return mCamera.vp;}
        virtual glm::vec3 getLightDirect() = 0;
        virtual glm::vec3 getLightColor() = 0;

    protected:
        lightState mLightState;
        lightCamera mCamera;
        allocManager::bufSptr mVpUniformBuf;
    };

    struct lightStateEx{
        bool shadowCasting{true}; //假定所有光源默认都是要产生阴影的
        float Range{1000.f}; //点光源和聚光灯光源影响的最大范围
        glm::vec3 colorTemperature{1.f}; //中性白光，不进行额外的色温调整
    };

} // yic

#endif //VKMMD_LIGHTBASE_H
