//
// Created by lenovo on 3/23/2024.
//

#include "vkRayTracing.h"

namespace yic {

    vkRayTracing &vkRayTracing::init(yic::vk_init *vkInit) {
        mDevice = vkInit->device();
        mPhysicalDevice = vkInit->physicalDevice();
        mGraphicsQueueIndex = vkInit->getGraphicsFamilyIndices();
        mGraphicsQueue = vkInit->graphicsQueue();
        vk::CommandPoolCreateInfo createInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, mGraphicsQueueIndex};
        vkCreate([&](){ mCommandPool = mDevice.createCommandPool(createInfo); }, " create command pool ");

        return *this;
    }

    vkRayTracing &vkRayTracing::initRayTracing(vkPmx::pmxModel pmxModel) {
        vk::PhysicalDeviceProperties2 properties2;
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties;
        properties2.pNext = &rtProperties;
        mPhysicalDevice.getProperties2(&properties2);

        mRtBuilder.init(mDevice, mGraphicsQueue, mCommandPool);
        mPmxModels.emplace_back(pmxModel);

        return *this;
    }

    vkRayTracing &vkRayTracing::createBottomLevelAS() {
        std::vector<rt::rayTracingKHR::vkBlasInput> allBlas;
        allBlas.reserve(mPmxModels.size());
        for(const auto& pmx : mPmxModels){
            auto blas = pmxToVkGeometryKHR(pmx);

            allBlas.emplace_back(blas);
        }

        mRtBuilder.buildBlas(allBlas,vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);

        return *this;
    }

    vkRayTracing &vkRayTracing::createTopLevelAS() {
        std::vector<vk::AccelerationStructureInstanceKHR> tlas;
        mObjInstances.emplace_back(glm::mat4 {1.f}, 0);
        tlas.reserve(mObjInstances.size());

        for(const auto& inst : mObjInstances){
            vk::AccelerationStructureInstanceKHR accelInstance{
                rt::toTransformMatrix(inst.transform), inst.objIndex,
                0xFF, //0xFF 表示所有光线都可以与此实例相交，只有当光线的掩码与实例的掩码进行按位与操作（&）的结果非0时，光线才会与此实例相交
                0, //表示所有实例使用相同的命中着色器组。这个值是用于访问Shader Binding Table（SBT）中特定的着色器组
                vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable, mRtBuilder.getBlasDeviceAddress(inst.objIndex)
            };
            tlas.emplace_back(accelInstance);
        }
        mRtBuilder.buildTlas(tlas, vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);//优先考虑光线追踪性能

        return *this;
    }

    vkRayTracing &vkRayTracing::createRtPipeline() {
        enum stageIndices{
            eRagGen, eMiss, eMiss2, eClosestHit, eShaderGroupCount
        };

        std::array<vk::PipelineShaderStageCreateInfo, eShaderGroupCount> stages{};
        stages[eRagGen] = vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eRaygenKHR, vkHelp::createShaderModule(mDevice, ke_q::loadFile("")), "main"};
        stages[eMiss] = vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eMissKHR, vkHelp::createShaderModule(mDevice, ke_q::loadFile("")), "main"};
        stages[eMiss2] = vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eMissKHR, vkHelp::createShaderModule(mDevice, ke_q::loadFile("")), "main"};
        stages[eClosestHit] = vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eClosestHitKHR, vkHelp::createShaderModule(mDevice, ke_q::loadFile("")), "main"};

        return *this;
    }


    rt::rayTracingKHR::vkBlasInput vkRayTracing::pmxToVkGeometryKHR(const vkPmx::pmxModel &pmx) {
        vk::DeviceAddress vertexAddress = ke_q::getBufDeviceAddress(mDevice, pmx.mVertexBuffer.buffer);
        vk::DeviceAddress indexAddress = ke_q::getBufDeviceAddress(mDevice, pmx.mIndexBuffer.buffer);

        auto maxPrimitiveCount = pmx.indices / 3;

        vk::AccelerationStructureGeometryTrianglesDataKHR triangles{
                vk::Format::eR32G32B32Sfloat, // X 32 Y 32 Z 32
                vertexAddress, sizeof (vkPmx::Vertex), pmx.vertices - 1,
                pmx.indexType, indexAddress
        };

        vk::AccelerationStructureGeometryKHR asGeom{
            vk::GeometryTypeKHR::eTriangles, triangles, vk::GeometryFlagBitsKHR::eOpaque
        };

        vk::AccelerationStructureBuildRangeInfoKHR range{
            maxPrimitiveCount, 0, 0, 0
        };

        rt::rayTracingKHR::vkBlasInput input{.asGeometry{asGeom}, .asBuildOffsetInfo{range}};

        return input;
    }



} // rt