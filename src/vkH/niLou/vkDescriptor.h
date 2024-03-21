//
// Created by lenovo on 12/24/2023.
//

#ifndef VULKAN_VKDESCRIPTOR_H
#define VULKAN_VKDESCRIPTOR_H

#include "log/Log.h"
#include "miku/vkIdGenerator.h"

namespace yic {

    class vkDescriptor : public nonCopyable{
    public:
        vkDescriptor(){
            mUniqueId = vkH::vkIdGenerator::get()->generatorUniqueId();
        }

        vkDescriptor& init(vk::Device device);
        vkDescriptor& increaseMaxSets(uint32_t i = 1){ mMaxSets += i; return *this; }
        vkDescriptor& resetIndex() { index = 0; return *this; }

        vkDescriptor& addDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
        vkDescriptor& addDescriptorSetLayout(const std::vector<std::pair<vk::DescriptorSetLayoutBinding, uint32_t>>& bindings);
        vkDescriptor& createDescriptorPool();
        vkDescriptor& createDescriptorSets();
        vkDescriptor& pushBackDesSets();

        vkDescriptor& addDescriptorSet(const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>>& info = {});
        vkDescriptor& updateDescriptorSet(const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>>& info = {});
        vkDescriptor& update(){
            mDevice.updateDescriptorSets(mWriteDescriptor, nullptr);
            return *this;
        }

        [[nodiscard]] inline auto& getId() const { return mUniqueId;}
        [[nodiscard]] inline auto& getDescriptorPool() const { return mDescriptorPool;}
        [[nodiscard]] auto& getDescriptorSetLayouts() const { return mDescriptorSetLayouts;}
        [[nodiscard]] auto& getDescriptorSets() const { return mDescriptorSets;}
        [[nodiscard]] inline auto& getPoolSize() const { return mPoolSize;}
        [[nodiscard]] inline auto& getWriteDescriptorSet() const { return mWriteDescriptor;}
        [[nodiscard]] inline auto getPipelineLayout() const {
            vk::PipelineLayout pipeLayout;
            vk::PipelineLayoutCreateInfo createInfo{{}, mDescriptorSetLayouts, {}};
            vkCreate([&](){pipeLayout = mDevice.createPipelineLayout(createInfo);}, "create pipeline layout");
            return pipeLayout;}
    private:
        static vk::Device mDevice;

        uint32_t mMaxSets{};
        uint32_t index{};
        std::string mUniqueId;
        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> mBindings{};
        std::vector<vk::DescriptorSetLayout> mDescriptorSetLayouts{};
        std::vector<vk::DescriptorPoolSize> mPoolSize{};
        vk::DescriptorPool mDescriptorPool{};
        std::vector<vk::DescriptorSet> mDescriptorSets{};
        std::vector<vk::WriteDescriptorSet> mWriteDescriptor{};
        std::vector<std::vector<vk::WriteDescriptorSet>> mVecWriteDesSets{};
    };

} // yic

#endif //VULKAN_VKDESCRIPTOR_H
