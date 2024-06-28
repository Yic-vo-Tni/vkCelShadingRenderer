//
// Created by lenovo on 1/13/2024.
//

#include "pmx_header.h"

namespace vkPmx {

    std::vector<std::unique_ptr<StagingBuffer>> StagingBuffer::mStagingBuffers;
    vk::Device StagingBuffer::mDevice;
    vk::PhysicalDevice StagingBuffer::mPhysicalDevice;
    vk::CommandPool StagingBuffer::mTransferCommandPool{};
    vk::Queue StagingBuffer::mGraphicsQueue;

    bool StagingBuffer::init(yic::vk_init *vkInit, const vk::CommandPool& transferPool) {
        mTransferCommandPool = transferPool;
        mDevice = vkInit->device();
        mPhysicalDevice = vkInit->physicalDevice();
        mGraphicsQueue = vkInit->graphicsQueue();

        return true;
    }

    bool StagingBuffer::Setup(vk::DeviceSize size)
    {
        if (size <= m_memorySize){ return true; }
        Clear();

        // Create Buffer
        auto bufInfo = vk::BufferCreateInfo().setSize(size).setUsage(vk::BufferUsageFlagBits::eTransferSrc);
        vkCreate([&](){ m_buffer = mDevice.createBuffer(bufInfo); }, "create staging buf");


        // Allocate Mmoery
        auto bufMemReq = mDevice.getBufferMemoryRequirements(m_buffer);
        uint32_t bufMemTypeIndex;

        bufMemTypeIndex = yic::vk_fn::getMemoryType(mPhysicalDevice, bufMemReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        auto memAllocInfo = vk::MemoryAllocateInfo()
                .setAllocationSize(bufMemReq.size)
                .setMemoryTypeIndex(bufMemTypeIndex);
        vkCreate([&](){ m_memory = mDevice.allocateMemory(memAllocInfo);}, "allocate staging buf memory");

        mDevice.bindBufferMemory(m_buffer, m_memory, 0);

        m_memorySize = uint32_t(bufMemReq.size);

        vk::CommandBufferAllocateInfo allocInfo{mTransferCommandPool, vk::CommandBufferLevel::ePrimary, 1};
        vkCreate([&](){ m_copyCommand = mDevice.allocateCommandBuffers(allocInfo).front();}, "allocate copy cmd");
        // Create Fence
        vkCreate([&](){ m_transferCompleteFence = mDevice.createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));}, "create staging buf fence");
        // Create Semaphore
        vkCreate([&](){ m_transferCompleteSemaphore = mDevice.createSemaphore(vk::SemaphoreCreateInfo());}, "create staging buf semaphore");

        return true;
    }

    void StagingBuffer::Wait(){
        if (m_transferCompleteFence){
            if (mDevice.waitForFences(m_transferCompleteFence, true, -1) != vk::Result::eSuccess){
                return;
            }
        }
    }

    void StagingBuffer::Clear(){
        Wait();

        mDevice.destroyFence(m_transferCompleteFence, nullptr);
        m_transferCompleteFence = nullptr;

        mDevice.destroySemaphore(m_transferCompleteSemaphore, nullptr);
        m_transferCompleteSemaphore = nullptr;
        m_waitSemaphore = nullptr;

        auto cmdPool = mTransferCommandPool;
        mDevice.freeCommandBuffers(cmdPool, 1, &m_copyCommand);
        m_copyCommand = nullptr;

        mDevice.freeMemory(m_memory, nullptr);
        m_memory = nullptr;

        mDevice.destroyBuffer(m_buffer, nullptr);
        m_buffer = nullptr;
    }

    bool StagingBuffer::CopyBuffer(vk::Buffer buffer, vk::DeviceSize size){
        m_copyCommand.begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

        auto copyRegion = vk::BufferCopy().setSize(size);
        m_copyCommand.copyBuffer(m_buffer, buffer, 1, &copyRegion);

        m_copyCommand.end();

        // Submit
        auto submitInfo = vk::SubmitInfo().setCommandBuffers(m_copyCommand).setSignalSemaphores(m_transferCompleteSemaphore);
        vk::PipelineStageFlags waitDstStage = vk::PipelineStageFlagBits::eTransfer;
        if (m_waitSemaphore){
            submitInfo.setWaitSemaphores(m_waitSemaphore).setPWaitDstStageMask(&waitDstStage);
        }

        mGraphicsQueue.submit(submitInfo, m_transferCompleteFence);
        m_waitSemaphore = m_transferCompleteSemaphore;

        return true;
    }



    bool StagingBuffer::getStagingBuffer(vk::DeviceSize memSize, StagingBuffer **outBuf){
        if (outBuf == nullptr)
            return false;


        for (auto& stBuf : mStagingBuffers)
        {
            if(mDevice.getFenceStatus(stBuf->m_transferCompleteFence) == vk::Result::eSuccess && memSize < stBuf->m_memorySize){
                if( mDevice.resetFences(1, &stBuf->m_transferCompleteFence) != vk::Result::eSuccess){ return false; }
                *outBuf = stBuf.get();
                return true;
            }
        }

        for (auto& stBuf : mStagingBuffers)
        {
            if (mDevice.getFenceStatus(stBuf->m_transferCompleteFence) == vk::Result::eSuccess){
                stBuf->Setup(memSize);
                if (mDevice.resetFences(1, &stBuf->m_transferCompleteFence) != vk::Result::eSuccess){ return false; }
                *outBuf = stBuf.get();
                return true;
            }
        }

        auto newStagingBuffer = std::make_unique<StagingBuffer>();
        newStagingBuffer->Setup(memSize);
        if(mDevice.resetFences(1, &newStagingBuffer->m_transferCompleteFence) != vk::Result::eSuccess){ return false; }

        *outBuf = newStagingBuffer.get();
        mStagingBuffers.emplace_back(std::move(newStagingBuffer));

        return true;
    }



} // vkPmx