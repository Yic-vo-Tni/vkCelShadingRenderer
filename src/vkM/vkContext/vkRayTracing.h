//
// Created by lenovo on 3/23/2024.
//

#ifndef VKMMD_VKRAYTRACING_H
#define VKMMD_VKRAYTRACING_H

#include "rayTracing/rayTracingKHR.h"

#include "pmx/pmx_header.h"
#include "scene/vkScene.h"
#include "ke_q/file_operation.h"
#include "vkInit/vk_init.h"
#include "vkCore/vkShader.h"

namespace yic {


    class vkRayTracing {
    public:
        vkRayTracing& init(vk_init* vkInit);
        vkRayTracing& initRayTracing(vkPmx::pmxModel pmxModel);
        vkRayTracing& createBottomLevelAS();
        vkRayTracing& createTopLevelAS();
        vkRayTracing& createRtPipeline();

    public:
        rt::rayTracingKHR::vkBlasInput pmxToVkGeometryKHR(const vkPmx::pmxModel& pmx);

    private:
        rt::rayTracingKHR mRtBuilder;

    private:
        vk::Device mDevice;
        vk::PhysicalDevice mPhysicalDevice;
        vk::Queue mGraphicsQueue;
        vk::CommandPool mCommandPool;
        uint32_t mGraphicsQueueIndex;

        std::vector<vkPmx::pmxModel> mPmxModels;
        std::vector<vkScene::objInstance> mObjInstances;
    };

} // rt

#endif //VKMMD_VKRAYTRACING_H
