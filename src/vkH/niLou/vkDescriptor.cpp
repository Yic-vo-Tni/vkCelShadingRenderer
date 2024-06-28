//
// Created by lenovo on 12/24/2023.
//

#include "vkDescriptor.h"

namespace yic {

    vk::Device vkDescriptor::mDevice;

    vkDescriptor &vkDescriptor::init(vk::Device device) {
        mDevice = device;
        return *this;
    }

    vkDescriptor& vkDescriptor::addDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding> &bindings) {
        for(const auto& bind : bindings){
            mPoolSize.emplace_back(bind.descriptorType, bind.descriptorCount);
        }
        mMaxSets++;
        mBindings.push_back(bindings);
        vk::DescriptorSetLayoutCreateInfo createInfo{{}, bindings};
        vkCreate([&](){mDescriptorSetLayouts.push_back(mDevice.createDescriptorSetLayout(createInfo));}, "create descriptor set layouts");
        return *this;
    }

    vkDescriptor& vkDescriptor::addDescriptorSetLayout(
            const std::vector<std::pair<vk::DescriptorSetLayoutBinding, uint32_t>> &bindings) {
        for(const auto& bind : bindings){
            mBindings.resize(mMaxSets + 1);
            if (bind.second != 0){
                mPoolSize.emplace_back(bind.first.descriptorType, bind.second);
            }
            mBindings[mMaxSets].push_back(bind.first);
        }
        vk::DescriptorSetLayoutCreateInfo createInfo{{}, mBindings[mMaxSets]};
        vkCreate([&](){mDescriptorSetLayouts.push_back(mDevice.createDescriptorSetLayout(createInfo));}, "create descriptor set layouts");
        mMaxSets++;
        return *this;
    }

    vkDescriptor &vkDescriptor::createDescriptorPool() {
        vk::DescriptorPoolCreateInfo createInfo{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, mMaxSets, mPoolSize};
        vkCreate([&](){mDescriptorPool = mDevice.createDescriptorPool(createInfo);}, "create descriptor pool");
        return *this;
    }

    vkDescriptor &vkDescriptor::createDescriptorSets() {
        vk::DescriptorSetAllocateInfo allocateInfo{mDescriptorPool, mDescriptorSetLayouts};
        vkCreate([&](){mDescriptorSets = mDevice.allocateDescriptorSets(allocateInfo);}, "allocate descriptor sets");
        return *this;
    }

    vkDescriptor &vkDescriptor::pushBackDesSets(uint32_t set) {
        vk::DescriptorSetAllocateInfo allocateInfo{mDescriptorPool, mDescriptorSetLayouts};
        vkCreate([&](){mDescriptorSets.push_back(mDevice.allocateDescriptorSets(allocateInfo)[set]);}, "allocate descriptor sets");

        return *this;
    }

    vkDescriptor& vkDescriptor::addDescriptorSet(const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> &info) {
        for(uint32_t i = 0; auto& bind : mBindings[index]){
            std::visit([&](auto&& arg){
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, vk::DescriptorBufferInfo>){
                    if (arg.buffer){
                        mWriteDescriptor.push_back(vk::WriteDescriptorSet{mDescriptorSets[index], bind.binding, 0,
                                                                          bind.descriptorType, {}, arg});
                    }
                } else if constexpr (std::is_same_v<T, vk::DescriptorImageInfo>){
                    if(arg.sampler){
                        mWriteDescriptor.push_back(vk::WriteDescriptorSet{mDescriptorSets[index], bind.binding, 0,
                                                                          bind.descriptorType, arg});
                    }
                }
            }, info[i]);
            i++;
        }
        index++;
        return *this;
    }

    vkDescriptor &vkDescriptor::updateDescriptorSet(
            const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> &info, uint32_t set) {
        mVecWriteDesSets.resize(index + 1);
        for(uint32_t i = 0; auto& bind : mBindings[set]){
            std::visit([&](auto&& arg){
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, vk::DescriptorBufferInfo>){
                    mVecWriteDesSets[index].push_back(vk::WriteDescriptorSet{mDescriptorSets[index], bind.binding, 0,
                                                                      bind.descriptorType, {}, arg});
                }else if constexpr (std::is_same_v<T, vk::DescriptorImageInfo>){
                    mVecWriteDesSets[index].push_back(vk::WriteDescriptorSet{mDescriptorSets[index], bind.binding, 0,
                                                                          bind.descriptorType, arg});
                }
            }, info[i]);
            i++;
        }
        mDevice.updateDescriptorSets(mVecWriteDesSets[index], nullptr);

        index++;
        return *this;
    }

    vkDescriptor &vkDescriptor::update(uint32_t index_,
                                       const std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> &info) {
        std::vector<vk::WriteDescriptorSet> writeDesSet;
        for(uint32_t i = 0; auto& bind : mBindings[0]){
            std::visit([&](auto&& arg){
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, vk::DescriptorBufferInfo>){
                    writeDesSet.push_back(vk::WriteDescriptorSet{mDescriptorSets[index_], bind.binding, 0,
                                                                 bind.descriptorType, {}, arg});
                }else if constexpr (std::is_same_v<T, vk::DescriptorImageInfo>){
                    writeDesSet.push_back(vk::WriteDescriptorSet{mDescriptorSets[index_], bind.binding, 0,
                                                                             bind.descriptorType, arg});
                }
            }, info[i]);
            i++;
        }

        mDevice.updateDescriptorSets(writeDesSet, nullptr);

        return *this;
    }

} // yic