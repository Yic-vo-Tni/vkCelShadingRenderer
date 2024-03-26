//
// Created by lenovo on 3/22/2024.
//


#include "vk_offscreen.h"

namespace yic {

    vk_offscreen &vk_offscreen::createOffscreenPipeline() {
        mOffScreenDescriptor.addDescriptorSetLayout({vk::DescriptorSetLayoutBinding{
            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
        }});

        graphicsPipelineGeneratorCombined pipelineGeneratorCombined{mDevice, mOffScreenDescriptor.getPipelineLayout(), mOffScreenRenderPass};
        pipelineGeneratorCombined.addShader(ke_q::loadFile(""), vk::ShaderStageFlagBits::eVertex);
        pipelineGeneratorCombined.addShader(ke_q::loadFile(""), vk::ShaderStageFlagBits::eFragment);

        mOffScreenGraphicsPipeline = pipelineGeneratorCombined.createGraphicsPipeline();

        return *this;
    }

    vk_offscreen &vk_offscreen::updateDescriptor() {
        vk::SamplerCreateInfo samplerCreateInfo{{}, vk::Filter::eLinear, vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear,
                                                vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge,
                                                0.f, vk::False, 1.f,
                                                vk::False, vk::CompareOp::eAlways,
                                                0.f, 0.f,
                                                vk::BorderColor::eIntOpaqueBlack, vk::False};
        vkCreate([&](){ mSampler = mDevice.createSampler(samplerCreateInfo);}, " create off-screen sampler ");

        mOffScreenDescriptor
                .createDescriptorPool()
                .createDescriptorSets()
                .addDescriptorSet({
                                          vk::DescriptorImageInfo{mSampler, {}, vk::ImageLayout::eShaderReadOnlyOptimal},
                                  })
                .update();



        return *this;
    }

} // yic