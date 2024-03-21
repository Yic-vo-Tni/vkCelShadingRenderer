//
// Created by lenovo on 1/11/2024.
//

#include "vk_saba2.h"

std::vector<std::unique_ptr<StagingBuffer>> StagingBuffer::mStagingBuffers;
vk::Device StagingBuffer::mDevice;
vk::PhysicalDevice StagingBuffer::mPhysicalDevice;
vk::CommandPool StagingBuffer::mTransferCommandPool;
vk::Queue StagingBuffer::mGraphicsQueue;

bool StagingBuffer::Setup(vk::DeviceSize size)
{
    vk::Result ret;

    if (size <= m_memorySize)
    {
        return true;
    }

    Clear();

    // Create Buffer
    auto bufInfo = vk::BufferCreateInfo()
            .setSize(size)
            .setUsage(vk::BufferUsageFlagBits::eTransferSrc);
    ret = mDevice.createBuffer(&bufInfo, nullptr, &m_buffer);
    if (vk::Result::eSuccess != ret)
    {
        std::cout << "Failed to create Staging Buffer.\n";
        return false;
    }

    // Allocate Mmoery
    auto bufMemReq = mDevice.getBufferMemoryRequirements(m_buffer);
    uint32_t bufMemTypeIndex;

    bufMemTypeIndex = yic::vk_fn::getMemoryType(mPhysicalDevice, bufMemReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    auto memAllocInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(bufMemReq.size)
            .setMemoryTypeIndex(bufMemTypeIndex);
    ret = mDevice.allocateMemory(&memAllocInfo, nullptr, &m_memory);
    if (vk::Result::eSuccess != ret)
    {
        std::cout << "Failed to allocate Staging Buffer Memory.\n";
        return false;
    }

    mDevice.bindBufferMemory(m_buffer, m_memory, 0);

    m_memorySize = uint32_t(bufMemReq.size);

    // Allocate Command Buffer
    auto cmdBufAllocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(mTransferCommandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);
    ret = mDevice.allocateCommandBuffers(&cmdBufAllocInfo, &m_copyCommand);
    if (vk::Result::eSuccess != ret)
    {
        std::cout << "Failed to allocate Staging Buffer Copy Command Buffer.\n";
        return false;
    }

    // Create Fence
    auto fenceInfo = vk::FenceCreateInfo()
            .setFlags(vk::FenceCreateFlagBits::eSignaled);
    ret = mDevice.createFence(&fenceInfo, nullptr, &m_transferCompleteFence);
    if (vk::Result::eSuccess != ret)
    {
        std::cout << "Failed to  create Staging Buffer Transfer Complete Fence.\n";
        return false;
    }

    // Create Semaphore
    auto semaphoreInfo = vk::SemaphoreCreateInfo();
    ret = mDevice.createSemaphore(&semaphoreInfo, nullptr, &m_transferCompleteSemaphore);
    if (vk::Result::eSuccess != ret)
    {
        std::cout << "Failed to  create Staging Buffer Transer Complete Semaphore.\n";
        return false;
    }

    return true;
}

void StagingBuffer::Wait()
{
    vk::Result ret;
    if (m_transferCompleteFence)
    {
        ret = mDevice.waitForFences(1, &m_transferCompleteFence, true, -1);
    }
}

void StagingBuffer::Clear()
{
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

bool StagingBuffer::CopyBuffer(vk::Buffer buffer, vk::DeviceSize size)
{
    vk::Result ret;

    auto cmdBufInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    ret = m_copyCommand.begin(&cmdBufInfo);

    auto copyRegion = vk::BufferCopy()
            .setSize(size);
    m_copyCommand.copyBuffer(m_buffer, buffer, 1, &copyRegion);

    m_copyCommand.end();

    // Submit
    auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&m_copyCommand)
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(&m_transferCompleteSemaphore);
    vk::PipelineStageFlags waitDstStage = vk::PipelineStageFlagBits::eTransfer;
    if (m_waitSemaphore)
    {
        submitInfo
                .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(&m_waitSemaphore)
                .setPWaitDstStageMask(&waitDstStage);
    }

    ret = mGraphicsQueue.submit(1, &submitInfo, m_transferCompleteFence);
    m_waitSemaphore = m_transferCompleteSemaphore;
    if (vk::Result::eSuccess != ret)
    {
        std::cout << "Failed to submit Copy Command Buffer.\n";
        return false;
    }

    return true;
}

bool StagingBuffer::CopyImage(
        vk::Image destImage,
        vk::ImageLayout imageLayout,
        uint32_t regionCount,
        vk::BufferImageCopy* regions
)
{
    vk::Result ret;

    auto cmdBufInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    ret = m_copyCommand.begin(&cmdBufInfo);


    auto subresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(regionCount)
            .setLayerCount(1);

    SetImageLayout(
            m_copyCommand,
            destImage,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal,
            subresourceRange
    );

    m_copyCommand.copyBufferToImage(
            m_buffer,
            destImage,
            vk::ImageLayout::eTransferDstOptimal,
            regionCount,
            regions
    );

    SetImageLayout(
            m_copyCommand,
            destImage,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageLayout::eTransferDstOptimal,
            imageLayout,
            subresourceRange
    );

    m_copyCommand.end();

    // Submit
    auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&m_copyCommand)
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(&m_transferCompleteSemaphore);
    vk::PipelineStageFlags waitDstStage = vk::PipelineStageFlagBits::eTransfer;
    if (m_waitSemaphore)
    {
        submitInfo
                .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(&m_waitSemaphore)
                .setPWaitDstStageMask(&waitDstStage);
    }

    ret = mGraphicsQueue.submit(1, &submitInfo, m_transferCompleteFence);
    m_waitSemaphore = m_transferCompleteSemaphore;
    if (vk::Result::eSuccess != ret)
    {
        std::cout << "Failed to submit Copy Command Buffer.\n";
        return false;
    }
    return true;
}

bool StagingBuffer::getStagingBuffer(AppContext& appContext, vk::DeviceSize memSize, StagingBuffer **outBuf){
    mDevice = appContext.m_device;
    mPhysicalDevice = appContext.m_gpu;
    mTransferCommandPool = appContext.m_transferCommandPool;
    mGraphicsQueue = appContext.m_graphicsQueue;

    if (outBuf == nullptr)
    {
        return false;
    }

    vk::Result ret;
    for (auto& stBuf : mStagingBuffers)
    {
        ret = appContext.m_device.getFenceStatus(stBuf->m_transferCompleteFence);
        if (vk::Result::eSuccess == ret && memSize < stBuf->m_memorySize)
        {
            if(mDevice.resetFences(1, &stBuf->m_transferCompleteFence) != vk::Result::eSuccess){
                vkError("failed to reset fences");
            }
            *outBuf = stBuf.get();
            return true;
        }
    }

    for (auto& stBuf : mStagingBuffers)
    {
        ret = appContext.m_device.getFenceStatus(stBuf->m_transferCompleteFence);
        if (vk::Result::eSuccess == ret)
        {
            if (!stBuf->Setup(memSize))
            {
                std::cout << "Failed to setup Staging Buffer.\n";
                return false;
            }
            if( mDevice.resetFences(1, &stBuf->m_transferCompleteFence) != vk::Result::eSuccess){
                vkError("failed to reset fences");
            }
            *outBuf = stBuf.get();
            return true;
        }
    }

    auto newStagingBuffer = std::make_unique<StagingBuffer>();
    if (!newStagingBuffer->Setup(memSize))
    {
        std::cout << "Failed to setup Staging Buffer.\n";
        newStagingBuffer->Clear();
        return false;
    }
    if( mDevice.resetFences(1, &newStagingBuffer->m_transferCompleteFence) != vk::Result::eSuccess){
        vkError("failed to reset fences");
    }
    *outBuf = newStagingBuffer.get();
    mStagingBuffers.emplace_back(std::move(newStagingBuffer));
    return true;
}


