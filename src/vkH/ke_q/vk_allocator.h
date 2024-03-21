//
// Created by lenovo on 12/24/2023.
//

#ifndef VULKAN_VK_ALLOCATOR_H
#define VULKAN_VK_ALLOCATOR_H

#include "miku/vk_fn.h"
#include "vkInit/vk_init.h"

namespace yic {

    struct Buffer {
        vk::Buffer buffer;
        vk::DeviceMemory deviceMemory;
    };

    struct Image{
        vk::Image image;
        vk::ImageView imageView;
        vk::Sampler sampler;
    };

    class vkAllocator {
    protected:
        using MemReqs = std::function<vk::MemoryRequirements()>;
        using BindMem = std::function<void()>;
        inline static constexpr vk::MemoryPropertyFlags defaultMemProperty = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        MemReqs defaultMemReqs = [this](){return vk::MemoryRequirements{mDevice.getBufferMemoryRequirements(mBuffer.buffer)};};
        MemReqs defaultStagingMemReqs = [this](){return vk::MemoryRequirements{mDevice.getBufferMemoryRequirements(mStagingBuffer.buffer)};};
        BindMem defaultBindMem = [this](){ mDevice.bindBufferMemory(mBuffer.buffer, mBuffer.deviceMemory, 0); };
        BindMem defaultStagingBindMem = [this](){ mDevice.bindBufferMemory(mStagingBuffer.buffer, mStagingBuffer.deviceMemory, 0); };
    public:
        vkAllocator() = default;
        static void init(vk_init* vkInit);

        template<typename T>
        vkAllocator& updateBuffer(const T& src) { memcpy(mData, &src, sizeof(src)); return *this;}
        vkAllocator& unmap();

        void allocMem(const MemReqs& memReqs, const BindMem& bindMem,
                      const vk::MemoryPropertyFlags& memProperty = defaultMemProperty);
        vk::DeviceMemory allocMem(const MemReqs& memReqs, const vk::MemoryPropertyFlags& memProperty = defaultMemProperty);

        void allocBuffer(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags usage,
                         const MemReqs& memReqs, const BindMem& bindMem, bool unmap = true);
        void allocBuffer(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags usage,
                         const MemReqs& memReqs, const BindMem& bindMem,
                         vk::MemoryPropertyFlags memPro, bool unmap = true);

        [[nodiscard]] inline auto& getData() const  { return mData;}
        [[nodiscard]] inline auto& getBuffer() const { return mBuffer.buffer;}
        [[nodiscard]] inline auto& getStagingBuffer() const { return mStagingBuffer.buffer;}
        [[nodiscard]] inline auto& getDeviceSize() const { return mDeviceSize;}

    protected:
        vkAllocator& createTempCmdBuf(){
            vk::CommandBufferAllocateInfo allocateInfo{mTransferCommandPool, vk::CommandBufferLevel::ePrimary, 1};
            mCmd = mDevice.allocateCommandBuffers(allocateInfo).front();

            vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
            mCmd.begin(beginInfo);

            return *this;
        }
        vkAllocator& submitTempCmdBuf(){
            mCmd.end();

            vk::SubmitInfo submitInfo{};
            submitInfo.setCommandBuffers(mCmd);
            mGraphicsQueue.submit(submitInfo);
            mGraphicsQueue.waitIdle();
            mDevice.free(mTransferCommandPool, mCmd);

            return *this;
        }

    protected:
        static vk::Device mDevice;
        static vk::PhysicalDevice mPhysicalDevice;
        static vk::CommandPool mTransferCommandPool;
        static vk::Queue mGraphicsQueue;

        vk::CommandBuffer mCmd;

        vk::DeviceSize mDeviceSize{};
        void* mData{nullptr};
        Buffer mBuffer;
        Buffer mStagingBuffer;
        bool mUnmap = true;

        void free() const{
            mDevice.destroy(mBuffer.buffer);
            mDevice.free(mBuffer.deviceMemory);
        }
    };

    class genericBufferManager : public vkAllocator{
    public:
        ~genericBufferManager() { if (mUnmap) unmap(); }

        MemReqs memReqs = [this](){return vk::MemoryRequirements{mDevice.getBufferMemoryRequirements(mBuffer.buffer)};};
        BindMem bindMem = [this](){ mDevice.bindBufferMemory(mBuffer.buffer, mBuffer.deviceMemory, 0); };

        genericBufferManager(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags usage, bool unmap = true){
            allocBuffer(deviceSize, data, usage, memReqs, bindMem, unmap);
        }
        genericBufferManager(vk::DeviceSize deviceSize, vk::BufferUsageFlags usage){
            allocBuffer(deviceSize, nullptr, usage, memReqs, bindMem, false);
        }
    };

} // yic

    using genericBufferManagerUptr = std::unique_ptr<yic::genericBufferManager>;

#endif //VULKAN_VK_ALLOCATOR_H























//        using bufferFun = std::function<vk::Buffer(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags usage)>;


//        std::function<vk::Buffer(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags usage)>
//                bufferX = [this](vk::DeviceSize deviceSize = {}, const void* data = {}, vk::BufferUsageFlags usage = {}){
//            if (!mBuffer.buffer){
//                Allocator::get()->allocBuffer(deviceSize, data, usage, false, &mData);
//            }
//            return mBuffer.buffer;
//        };