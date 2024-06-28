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
        vk::PhysicalDeviceProperties2 properties2{};
        properties2.pNext = &mRtProperties;
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
        enum stageIndices{ eRagGen, eMiss, eMiss2, eClosestHit, eShaderGroupCount };

        auto createShaderStageInfo = [&](vk::ShaderStageFlagBits flags, const std::string& path){
            return vk::PipelineShaderStageCreateInfo{{}, flags, vkHelp::createShaderModule(mDevice, ke_q::loadFile(path)), "main"};};
        std::array<vk::PipelineShaderStageCreateInfo, eShaderGroupCount> stages{};
        stages[eRagGen] = createShaderStageInfo(vk::ShaderStageFlagBits::eRaygenKHR, "gen_ray");
        stages[eMiss] = createShaderStageInfo(vk::ShaderStageFlagBits::eMissKHR, "miss_ray");
        stages[eMiss2] = createShaderStageInfo(vk::ShaderStageFlagBits::eMissKHR, "shadowMiss_ray");
        stages[eClosestHit] = createShaderStageInfo(vk::ShaderStageFlagBits::eClosestHitKHR, "hit_ray");

        using info = vk::RayTracingShaderGroupCreateInfoKHR();
        auto shaderGroup = [&](stageIndices indices){ return vk::RayTracingShaderGroupCreateInfoKHR().setGeneralShader(indices); };
        mRtShaderGroups.emplace_back(shaderGroup(eRagGen));
        mRtShaderGroups.emplace_back(shaderGroup(eMiss));
        mRtShaderGroups.emplace_back(shaderGroup(eMiss2));
        mRtShaderGroups.emplace_back(shaderGroup(eClosestHit));

        using b = vk::ShaderStageFlagBits;
        vk::PushConstantRange pushConstant{b::eRaygenKHR | b::eClosestHitKHR | b::eMissKHR, 0, sizeof (PushConstantRay)};
        mRtPipelineLayout = mDevice.createPipelineLayout(vk::PipelineLayoutCreateInfo().setPushConstantRanges(pushConstant));

        vkCreate([&](){ mRtPipeline = mDevice.createRayTracingPipelineKHR({}, {}, vk::RayTracingPipelineCreateInfoKHR().setStages(stages)
                                                                                                                            .setGroups(mRtShaderGroups)
                                                                                                                            .setLayout(mRtPipelineLayout)
                                                                                                                            .setMaxPipelineRayRecursionDepth(2),
                                                                          nullptr, gl::dispatchLoaderDynamic_).value;}, "create Rt pipeline");
        for(auto& stage : stages){ mDevice.destroy(stage.module); }

        return *this;
    }

    vkRayTracing &vkRayTracing::createRtSBT() {
        uint32_t missCount{2}, hitCount{1};
        auto handleCount = hitCount + missCount + 1;
        uint32_t handleSize = mRtProperties.shaderGroupHandleAlignment;

        uint32_t handleSizeAligned = vkH::align_up(handleSizeAligned, mRtProperties.shaderGroupHandleAlignment);

        mRgenRegion.setStride(vkH::align_up(handleSizeAligned, mRtProperties.shaderGroupBaseAlignment)).setSize(mRgenRegion.stride);
        mMissRegion.setStride(handleSizeAligned).setSize( vkH::align_up(missCount * handleSizeAligned, mRtProperties.shaderGroupBaseAlignment));
        mHitRegion.setStride(handleSizeAligned).setSize(vkH::align_up(hitCount * handleSizeAligned, mRtProperties.shaderGroupBaseAlignment));

        auto handles = mDevice.getRayTracingShaderGroupHandleKHR<std::vector<uint8_t>>(mRtPipeline, 0, handleCount, gl::dispatchLoaderDynamic_);

        auto sbtSize = mRgenRegion.size + mMissRegion.size + mHitRegion.size + mCallRedion.size;
        using bufUsage = vk::BufferUsageFlagBits;
        mRtSBTBuffer = allocManager::build::bufAccelAddressUptr(sbtSize, bufUsage::eTransferSrc | bufUsage::eShaderDeviceAddress | bufUsage::eShaderBindingTableKHR);

        vk::BufferDeviceAddressInfo info{mRtSBTBuffer->getBuffer()};
        auto sbtAddr = mDevice.getBufferAddress(info, gl::dispatchLoaderDynamic_);
        mRgenRegion.deviceAddress = sbtAddr;
        mMissRegion.deviceAddress = sbtAddr + mRgenRegion.size;
        mHitRegion.deviceAddress = sbtAddr + mRgenRegion.size + mMissRegion.size;

        auto getHandle = [&](int i) { return handles.data() + i * handleSize; };

        uint32_t handleIdx{0};
        uint32_t offset{0};
        mRtSBTBuffer->updateBuffer(getHandle(handleIdx++), handleSize, offset);

        offset = mRgenRegion.size;
        for(uint32_t c = 0; c < missCount; c++){
            mRtSBTBuffer->updateBuffer(getHandle(handleIdx++), handleSize, offset);
            offset += mMissRegion.stride;
        }

        offset = mRgenRegion.size + mMissRegion.size;
        for(uint32_t c = 0; c < hitCount; c++){
            mRtSBTBuffer->updateBuffer(getHandle(handleIdx++), handleSize, offset);
            offset += mHitRegion.stride;
        }

        mRtSBTBuffer->unmap();

        return *this;
    }


    rt::rayTracingKHR::vkBlasInput vkRayTracing::pmxToVkGeometryKHR(const vkPmx::pmxModel &pmx) {
        vk::DeviceAddress vertexAddress = ke_q::getBufDeviceAddress(mDevice, pmx.mVertexBuffer.buffer);
        vk::DeviceAddress indexAddress = ke_q::getBufDeviceAddress(mDevice, pmx.mIndexBuffer.buffer);

        auto maxPrimitiveCount = pmx.indices / 3;

        vk::AccelerationStructureGeometryTrianglesDataKHR triangles{
                vk::Format::eR32G32B32Sfloat, // X 32 Y 32 Z 32
                vertexAddress, sizeof (yicVertex), pmx.vertices - 1,
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