//
// Created by lenovo on 3/23/2024.
//

#include "rayTracingKHR.h"

#undef MemoryBarrier

namespace rt {

    void rayTracingKHR::init(vk::Device device, vk::Queue graphicsQueue,
                             vk::CommandPool commandPool) {
        mDevice = device;
        mGraphicsQueue = graphicsQueue;
        mCommandPool = commandPool;
    }

    void rayTracingKHR::buildBlas(const std::vector<vkBlasInput> &input, vk::BuildAccelerationStructureFlagsKHR flags) {
        auto inputCount = input.size();
        vk::DeviceSize asTotalSize{};  //BLAS所需总内存大小
        vk::DeviceSize maxScratchSize{};  //构建时，临时存储的最大缓冲区
        uint32_t nCompactions{};   //请求压缩BLAS的数量

        std::vector<buildAccelerationStructure> buildAs(inputCount);

        for(uint32_t i = 0; i < inputCount; i++){
            buildAs[i].buildInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)  //底层加速
                                .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)  //构建，而非更新现有的加速结构
                                .setFlags(input[i].flags | flags)    //各种优化标志，可能包括减少内存使用
                                .setPGeometries(input[i].asGeometry.data())  //几何信息
                                .setGeometryCount(static_cast<uint32_t>(input[i].asGeometry.size()));
            buildAs[i].rangeInfo = input[i].asBuildOffsetInfo.data();

            std::vector<uint32_t> maxPrimCount(input[i].asBuildOffsetInfo.size());
            for(auto t = 0; t < input[i].asBuildOffsetInfo.size(); t++){
                maxPrimCount[t] = input[i].asBuildOffsetInfo[t].primitiveCount; // 计算每个BLAS中几何体的最大原始图元数量
                                                                                // 原始图元通常指三角形，这里根据输入确定每个几何体的三角形数量
            }

            mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, &buildAs[i].buildInfo, maxPrimCount.data(), &buildAs[i].sizeInfo, gl::dispatchLoaderDynamic_);
            //为每个BLAS计算所需内存大小，包括BLAS的临时数据

            asTotalSize += buildAs[i].sizeInfo.accelerationStructureSize; //计算BLAS所需的总内存大小
            maxScratchSize = std::max(maxScratchSize, buildAs[i].sizeInfo.buildScratchSize); //更新临时最大缓冲区
            nCompactions += hasFlag(buildAs[i].buildInfo.flags, vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction); //检车是否设置压缩标志，减少BLAS的内存占用，如果设置了，nCompactions 计数器增加，这影响是否需要创建查询池来获取压缩后的大小
        }

        auto scratchBuf = allocManager::build::bufAccelAddressUptr(maxScratchSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eStorageBuffer);//用于BLAS构建和压缩过程中的临时数据,包括作为加速结构存储和普通的存储缓冲区
        vk::BufferDeviceAddressInfo bufferInfo{scratchBuf->getBuffer()};
        vk::DeviceAddress scratchAddress = mDevice.getBufferAddress(bufferInfo); //获取临时缓冲区的设备地址，这个地址会在后面用于指定构建和压缩过程中的临时数据位置

        vk::QueryPool queryPool{};
        if (nCompactions > 0){
            vk::QueryPoolCreateInfo createInfo{{}, vk::QueryType::eAccelerationStructureCompactedSizeKHR};
            vkCreate([&](){ queryPool = mDevice.createQueryPool(createInfo);}, "create query pool", 1);
        }  //如果有BLAS设置了允许压缩，创建一个查询池（query pool）。查询池用于存储每个需要压缩的BLAS的压缩后大小

        std::vector<uint32_t> indices{};
        vk::DeviceSize batchSize{};
        vk::DeviceSize batchLimit{256'000'000};
        for(uint32_t i = 0; i < inputCount; i++){
            indices.push_back(i);
            batchSize += buildAs[i].sizeInfo.accelerationStructureSize; //batchSize 累加当前BLAS的大小，以确定何时达到批次处理的上限

            if (batchSize >= batchLimit || i == inputCount - 1){
                auto cmd = createTempCmdBuf();
                cmdCreateBlas(cmd, indices, buildAs, scratchAddress, queryPool);
                submitTempCmdBuf(cmd);  //创建临时命令缓冲区并调用函数来构建BLAS。然后提交命令缓冲区以执行构建操作

                if (queryPool){
                    auto cmdBuf = createTempCmdBuf();
                    cmdCompactBlas(cmd, indices, buildAs, queryPool);
                    submitTempCmdBuf(cmdBuf);  //创建一个临时命令缓冲区，然后调用 cmdCompactBlas 函数来压缩BLAS

                    destroyNonCompacted(indices, buildAs); //销毁未被压缩的原始BLAS，释放资源
                }

                batchSize = 0;
                indices.clear();  //重置 batchSize 和 indices 以准备下一个批次的构建
            }
        }

        for(auto& b : buildAs){
            mBlas.emplace_back(b.as);
        }

        vkDestroyAll(mDevice, queryPool);
    }

    void rayTracingKHR::buildTlas(const std::vector<vk::AccelerationStructureInstanceKHR> &instances,
                                  vk::BuildAccelerationStructureFlagsKHR flags, bool update) {
        auto countInstance = instances.size();

        auto cmd = createTempCmdBuf();
        auto instanceBuf = allocManager::build::bufAccelAddressUptr(sizeof (vk::AccelerationStructureInstanceKHR) * instances.size(),
                                                                    vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

        vk::BufferDeviceAddressInfo bufferInfo{instanceBuf->getBuffer()};
        vk::DeviceSize instBufAddr{mDevice.getBufferAddress(bufferInfo, gl::dispatchLoaderDynamic_)};

        vk::MemoryBarrier barrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eAccelerationStructureWriteKHR};
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, {}, barrier, {}, {});

        yic::Buffer scratchBuf;
        cmdCreateTlas(cmd, countInstance, instBufAddr, scratchBuf, flags, update);

        submitTempCmdBuf(cmd);

        instanceBuf->free();
        mDevice.destroy(scratchBuf.buffer);
    }

    void rayTracingKHR::cmdCompactBlas(vk::CommandBuffer cmd, std::vector<uint32_t> indices,
                                       std::vector<buildAccelerationStructure> &buildAs, vk::QueryPool queryPool) {
        //这个函数 cmdCompactBlas 主要执行的是针对底层加速结构（BLAS）的压缩操作，旨在减少它们占用的内存空间，从而优化性能。
        uint32_t queryCount{};

        std::vector<vk::DeviceSize> compactSizes(indices.size()); //存储每个加速结构压缩后的大小
        vkGetQueryPoolResults(mDevice, queryPool, 0, (uint32_t)compactSizes.size(), compactSizes.size() * sizeof(VkDeviceSize),
                              compactSizes.data(), sizeof(VkDeviceSize), VK_QUERY_RESULT_WAIT_BIT);
        //从 queryPool 中检索查询结果，即每个加速结构压缩后的大小，将结果存储在 compactSizes 中。这里使用 VK_QUERY_RESULT_WAIT_BIT 标志表示，如果结果尚未就绪，函数将等待直到结果可用

        for(auto index : indices){ //被压缩的加速结构在 buildAs 向量中的索引
            buildAs[index].cleanupAs = buildAs[index].as;
            buildAs[index].sizeInfo.accelerationStructureSize = compactSizes[queryCount++];  //更新加速结构的预期大小为压缩后的大小, queryCount 作为索引

            vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
            asCreateInfo.setSize(buildAs[index].sizeInfo.accelerationStructureSize)  //压缩后的大小
                    .setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
            buildAs[index].as = allocFn::accel::createAcceleration(asCreateInfo); //创建新的加速结构，替换原来的加速结构

            vk::CopyAccelerationStructureInfoKHR copyInfo{
                buildAs[index].buildInfo.dstAccelerationStructure, buildAs[index].as.accel, vk::CopyAccelerationStructureModeKHR::eCompact
            };  //指定源加速结构、目标加速结构和拷贝模式为压缩模式（eCompact）
            cmd.copyAccelerationStructureKHR(copyInfo, gl::dispatchLoaderDynamic_);

            //优化方面
            //对于动态变化的内容，如会移动或变形的人物模型，频繁地压缩加速结构可能并不是最佳选择   常见的优化方法是优化加速结构的更新流程，如只更新变化的部分、使用实例化来减少重复内容的开销等
            //对于静态的场景内容，如不会变化的建筑、地形等，使用压缩加速结构可以显著减少它们在GPU内存中的占用
        }
    }

    void rayTracingKHR::cmdCreateBlas(vk::CommandBuffer cmd, std::vector<uint32_t> indices,
                                      std::vector<buildAccelerationStructure> &buildAs,
                                      vk::DeviceAddress scratchAddress, vk::QueryPool queryPool) {
        //索引是指定哪些被构建为加速结构，假如buildAs有5个，但是索引为1,3，则只有1和3构建加速结构
        if (queryPool){
            mDevice.resetQueryPool(queryPool, 0, indices.size());
        }
        //querypool用来查询加速结构构建完后的信息，如加速结构的最终大小，用于优化资源分配和管理

        uint32_t queryCount{};

        for(const auto& index : indices){
            vk::AccelerationStructureCreateInfoKHR createInfoKhr{};
            createInfoKhr.setType(vk::AccelerationStructureTypeKHR::eBottomLevel).setSize(buildAs[index].sizeInfo.accelerationStructureSize);

            buildAs[index].as = allocFn::accel::createAcceleration(createInfoKhr);
            buildAs[index].buildInfo.setDstAccelerationStructure(buildAs[index].as.accel)
                                    .setScratchData(scratchAddress);

            cmd.buildAccelerationStructuresKHR(buildAs[index].buildInfo, buildAs[index].rangeInfo, gl::dispatchLoaderDynamic_);  //构建加速结构

            vk::MemoryBarrier barrier{vk::AccessFlagBits::eAccelerationStructureWriteKHR, vk::AccessFlagBits::eAccelerationStructureReadKHR};
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                {}, barrier, {}, {});

            if (queryPool){
                cmd.writeAccelerationStructuresPropertiesKHR(buildAs[index].buildInfo.dstAccelerationStructure, vk::QueryType::eAccelerationStructureCompactedSizeKHR, queryPool, queryCount++, gl::dispatchLoaderDynamic_);
            }
        }
    }

    void rayTracingKHR::cmdCreateTlas(vk::CommandBuffer cmd, uint32_t countInst, vk::DeviceAddress instBufAddr,
                                      yic::Buffer &scratchBuf, vk::BuildAccelerationStructureFlagsKHR flags, bool update) {
        vk::AccelerationStructureGeometryInstancesDataKHR instancesDataKhr{{}, instBufAddr};
        vk::AccelerationStructureGeometryKHR tlasGeom{vk::GeometryTypeKHR::eInstances, instancesDataKhr};

        vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{
            vk::AccelerationStructureTypeKHR::eTopLevel, flags,
            update ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild,
            {}, {},tlasGeom, {}
        };

        auto sizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo, countInst, gl::dispatchLoaderDynamic_);

#ifdef VK_NV_ray_tracing_motion_blur
        vk::AccelerationStructureMotionInfoNV motionInfoNv{countInst};
#endif

        if (!update){
            auto createInfo = vk::AccelerationStructureCreateInfoKHR()
                    .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
                    .setSize(sizeInfo.accelerationStructureSize);

            mTlas = allocManager::fn::createAccel(createInfo);
        }

        scratchBuf.buffer = allocManager::build::bufAccelAddressUptr(sizeInfo.buildScratchSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress)->getBuffer();

        vk::BufferDeviceAddressInfo bufAddrInfo{scratchBuf.buffer};
        vk::DeviceAddress scratchAddr = mDevice.getBufferAddress(bufAddrInfo, gl::dispatchLoaderDynamic_);

        buildInfo.setSrcAccelerationStructure(update ? mTlas.accel : VK_NULL_HANDLE)
                .setDstAccelerationStructure(mTlas.accel)
                .setScratchData(scratchAddr);

        vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfoKhr{countInst, 0, 0, 0};
        //const vk::AccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &buildRangeInfoKhr;

        cmd.buildAccelerationStructuresKHR(buildInfo, &buildRangeInfoKhr, gl::dispatchLoaderDynamic_);
    }


    vk::DeviceAddress rayTracingKHR::getBlasDeviceAddress(uint32_t blasID) {
        assert(size_t(blasID) < mBlas.size());
        vk::AccelerationStructureDeviceAddressInfoKHR addressInfo{mBlas[blasID].accel};
        return mDevice.getAccelerationStructureAddressKHR(addressInfo, gl::dispatchLoaderDynamic_);
    }

    void rayTracingKHR::destroyNonCompacted(std::vector<uint32_t> indices,
                                            std::vector<buildAccelerationStructure> &buildAs) {
        for(auto& i : indices){
            allocFn::accel ::destroy(buildAs[i].cleanupAs);
        }
    }


    vk::CommandBuffer rayTracingKHR::createTempCmdBuf() {
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, 1};
        vk::CommandBuffer cmd = mDevice.allocateCommandBuffers(allocateInfo).front();

        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        cmd.begin(beginInfo);

        return cmd;
    }

    void rayTracingKHR::submitTempCmdBuf(vk::CommandBuffer& cmd) {
        cmd.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmd);
        mGraphicsQueue.submit(submitInfo);
        mGraphicsQueue.waitIdle();
        mDevice.free(mCommandPool, cmd);
    }

} // rt