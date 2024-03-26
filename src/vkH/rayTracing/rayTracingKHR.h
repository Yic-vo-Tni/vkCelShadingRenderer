//
// Created by lenovo on 3/23/2024.
//

#ifndef VKMMD_RAYTRACINGKHR_H
#define VKMMD_RAYTRACINGKHR_H

#include "ke_q/vk_allocator.h"

namespace rt {

    class rayTracingKHR {
    public:
        struct vkBlasInput
        {
            // Data used to build acceleration structure geometry
            std::vector<vk::AccelerationStructureGeometryKHR>       asGeometry;
            std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asBuildOffsetInfo;
            vk::BuildAccelerationStructureFlagsKHR                  flags{0};
        };

        struct buildAccelerationStructure{
            vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};  //BLAS的构建信息
            vk::AccelerationStructureBuildSizesInfoKHR sizeInfo{};  //构建每个BLAS所需的最大原始图元数量
            const vk::AccelerationStructureBuildRangeInfoKHR* rangeInfo{};
            yic::AccelKhr as;
            yic::AccelKhr cleanupAs;
        };

    public:
        void init(vk::Device device, vk::Queue graphicsQueue, vk::CommandPool commandPool);
        void buildBlas(const std::vector<vkBlasInput>& input, vk::BuildAccelerationStructureFlagsKHR flags);
        void buildTlas(const std::vector<vk::AccelerationStructureInstanceKHR>& instances, vk::BuildAccelerationStructureFlagsKHR flags,
                       bool update = false);

        vk::DeviceAddress getBlasDeviceAddress(uint32_t blasID);
    private:
        void cmdCreateBlas(vk::CommandBuffer cmd, std::vector<uint32_t> indices,
                           std::vector<buildAccelerationStructure>& buildAs,
                           vk::DeviceAddress scratchAddress, vk::QueryPool queryPool);
        void cmdCompactBlas(vk::CommandBuffer cmd, std::vector<uint32_t> indices,
                            std::vector<buildAccelerationStructure>& buildAs, vk::QueryPool queryPool);

        void cmdCreateTlas(vk::CommandBuffer cmd, uint32_t countInst, vk::DeviceAddress instBufAddr,
                           yic::Buffer& scratchBuf, vk::BuildAccelerationStructureFlagsKHR flags, bool update = false);

        vk::CommandBuffer createTempCmdBuf();
        void submitTempCmdBuf(vk::CommandBuffer& cmd);
        bool hasFlag(vk::Flags< vk::BuildAccelerationStructureFlagBitsKHR> item, vk::Flags< vk::BuildAccelerationStructureFlagBitsKHR> flag) { return (item & flag) == flag; }

        void destroyNonCompacted(std::vector<uint32_t> indices, std::vector<buildAccelerationStructure>& buildAs);

    protected:
        std::vector<yic::AccelKhr> mBlas;
        yic::AccelKhr mTlas;
    private:
        vk::Device mDevice;
        vk::Queue mGraphicsQueue;
        vk::CommandPool mCommandPool;
    };

    inline vk::TransformMatrixKHR toTransformMatrix(glm::mat4 matrix){
        glm::mat4 temp = glm::transpose(matrix);
        vk::TransformMatrixKHR matrixKhr;
        memcpy(&matrixKhr, &temp, sizeof (vk::TransformMatrixKHR));
        return matrixKhr;
    }

} // rt

#endif //VKMMD_RAYTRACINGKHR_H
