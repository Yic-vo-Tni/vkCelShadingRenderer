//
// Created by lenovo on 12/24/2023.
//

#include "vk_allocator.h"

namespace yic {

    vk::Device vkAllocator::mDevice;
    vk::PhysicalDevice vkAllocator::mPhysicalDevice;
    vk::CommandPool vkAllocator::mTransferCommandPool;
    vk::Queue vkAllocator::mGraphicsQueue;

    void vkAllocator::init(yic::vk_init *vkInit) {
        mPhysicalDevice = vkInit->physicalDevice();
        mDevice = vkInit->device();
        mGraphicsQueue = vkInit->getTransferQueue();
        auto transferCmdPoolInfo = vk::CommandPoolCreateInfo().setQueueFamilyIndex(vkInit->getTransferQueueFamily())
                .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        mTransferCommandPool = mDevice.createCommandPool(transferCmdPoolInfo);
    }

    void vkAllocator::allocBuffer(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags usage,
                                  const yic::vkAllocator::MemReqs &memReqs, const yic::vkAllocator::BindMem &bindMem,
                                  bool unmap) {
        vk::BufferCreateInfo bufferInfo{
                {}, deviceSize, usage, vk::SharingMode::eExclusive
        };
        mDeviceSize = deviceSize;
        mBuffer.buffer = mDevice.createBuffer(bufferInfo);

        allocMem(memReqs, bindMem);

        mData = mDevice.mapMemory(mBuffer.deviceMemory, 0, deviceSize);
        if (data) memcpy(mData, data, deviceSize);
        if (unmap) { this->unmap(); mUnmap = false; }
    }

    void vkAllocator::allocBufferDeviceAddress(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags usage,
                                               const yic::vkAllocator::MemReqs &memReqs,
                                               const yic::vkAllocator::BindMem &bindMem, bool unmap) {
        vk::BufferCreateInfo bufferInfo{
                {}, deviceSize, usage, vk::SharingMode::eExclusive
        };
        mDeviceSize = deviceSize;
        mBuffer.buffer = mDevice.createBuffer(bufferInfo);

        vk::MemoryAllocateFlagsInfo flagsInfo{vk::MemoryAllocateFlagBits::eDeviceAddress};
        allocMem(memReqs, bindMem, flagsInfo);

        mData = mDevice.mapMemory(mBuffer.deviceMemory, 0, deviceSize);
        if (data) memcpy(mData, data, deviceSize);
        if (unmap) { this->unmap(); mUnmap = false; }
    }

    void vkAllocator::allocBuffer(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags usage,
                                  const yic::vkAllocator::MemReqs &memReqs, const yic::vkAllocator::BindMem &bindMem,
                                  vk::MemoryPropertyFlags memPro, bool unmap) {
        vk::BufferCreateInfo bufferInfo{
                {}, deviceSize, usage, vk::SharingMode::eExclusive
        };
        mStagingBuffer.buffer = mDevice.createBuffer(bufferInfo);

        mStagingBuffer.deviceMemory = allocMem(defaultStagingMemReqs, defaultMemProperty);
        mDevice.bindBufferMemory(mStagingBuffer.buffer, mStagingBuffer.deviceMemory, 0);

        mData = mDevice.mapMemory(mStagingBuffer.deviceMemory, 0, deviceSize);
        memcpy(mData, data, deviceSize);
        mDevice.unmapMemory(mStagingBuffer.deviceMemory);

        allocMem(memReqs, bindMem, memPro);
    }

    void vkAllocator::allocMem(const yic::vkAllocator::MemReqs &memReqs, const yic::vkAllocator::BindMem &bindMem, const vk::MemoryPropertyFlags& memProperty) {
        vk::MemoryRequirements memoryRequirements = memReqs();
        vk::MemoryAllocateInfo allocateInfo{
                memoryRequirements.size,
                vk_fn::getMemoryType(mPhysicalDevice, memoryRequirements.memoryTypeBits, memProperty)
        };
        mBuffer.deviceMemory = mDevice.allocateMemory(allocateInfo);
        bindMem();
    }

    void vkAllocator::allocMem(const yic::vkAllocator::MemReqs &memReqs, const yic::vkAllocator::BindMem &bindMem,
                               vk::MemoryAllocateFlagsInfo flagsInfo, const vk::MemoryPropertyFlags &memProperty) {
        vk::MemoryRequirements memoryRequirements = memReqs();
        vk::MemoryAllocateInfo allocateInfo{
                memoryRequirements.size,
                vk_fn::getMemoryType(mPhysicalDevice, memoryRequirements.memoryTypeBits, memProperty), &flagsInfo
        };
        mBuffer.deviceMemory = mDevice.allocateMemory(allocateInfo);
        bindMem();
    }

    vk::DeviceMemory vkAllocator::allocMem(const yic::vkAllocator::MemReqs &memReqs, const vk::MemoryPropertyFlags &memProperty) {
        vk::MemoryRequirements memoryRequirements = memReqs();
        vk::MemoryAllocateInfo allocateInfo{
                memoryRequirements.size,
                vk_fn::getMemoryType(mPhysicalDevice, memoryRequirements.memoryTypeBits, memProperty)
        };
        return vk::DeviceMemory{mDevice.allocateMemory(allocateInfo)};
    }

    vkAllocator &vkAllocator::unmap() {
        mDevice.unmapMemory(mBuffer.deviceMemory);
        return *this;
    }


    /////////////////////////////////
















//    void Allocator::copyBuffer(yic::Buffer &srcBuffer, yic::Buffer &dstBuffer, vk::DeviceSize size, vk::Queue queue,
//                               vk::CommandBuffer cmd) {
//        vk::BufferCopy copy{
//            0, 0, size
//        };
//        cmd.copyBuffer(srcBuffer.buffer, dstBuffer.buffer, 1, &copy);
//    }


//    Buffer Allocator::allocBuffer(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags usage,
//                                  vk::MemoryPropertyFlags properties) {
//        Buffer mBuffer;
//        vk::BufferCreateInfo bufferInfo{
//                {}, deviceSize, usage, vk::SharingMode::eExclusive
//        };
//        mBuffer.buffer = mDevice.createBuffer(bufferInfo);
//
//        vk::MemoryRequirements memReqs{mDevice.getBufferMemoryRequirements(mBuffer.buffer)};
//
//        vk::MemoryAllocateInfo allocateInfo{
//                memReqs.size, vk_fn::getMemoryType(mPhysicalDevice, memReqs.memoryTypeBits,
//                                                   properties),
//        };
//
//        mBuffer.deviceMemory = mDevice.allocateMemory(allocateInfo);
//        mDevice.bindBufferMemory(mBuffer.buffer, mBuffer.deviceMemory, 0);
//        return mBuffer;
//    }

} // yic