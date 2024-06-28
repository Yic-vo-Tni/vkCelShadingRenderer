//
// Created by lenovo on 4/29/2024.
//

#include "volumeFog.h"

namespace yic {

    void volumeFog::init(vk::Device device, vk::PhysicalDevice physicalDevice) {
        mDevice = device;
        mPhysicalDevice = physicalDevice;

        create3DPerlinNoise();
        create3dPerlinNoiseImage();
    }

    void volumeFog::create(vk::RenderPass renderPass) {
        mRenderPass = renderPass;

        createFogPipeline();
        createFogDescriptor();
    }

    void volumeFog::draw(const vk::CommandBuffer &cmd) {
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mFogPipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mFogPipelineLayout, 0,
                               mFogDescriptor.getDescriptorSets(), nullptr);
        cmd.draw(6, 1, 0, 0);
    }

    void volumeFog::create3DPerlinNoise() {
        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFrequency(0.01f);

        m3DPerlinNoise.resize(mNoiseSize * mNoiseSize * depth);

        for(int index = 0, z = 0; z < depth; z++){
            for(int y = 0; y < mNoiseSize; y++){
                for(int x = 0; x < mNoiseSize; x++){
                    m3DPerlinNoise[index] = (noise.GetNoise((float)x, (float)y, (float)z) + 0.8f) * (float )0.4;

                    index ++;
                }
            }
        }

        mBlur3DPerlinNoise.resize(m3DPerlinNoise.size());
    }

    void volumeFog::create3dPerlinNoiseImage() {
        vk::ImageCreateInfo createInfo{
                {}, vk::ImageType::e3D, vk::Format::eR32Sfloat, {mNoiseSize, mNoiseSize, depth},
                1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined
        };

        vkCreate([&](){ mPerlinNoise = mDevice.createImage(createInfo); }, "create mPerlinNoise image");

        ///
        vk::MemoryRequirements memReqs{mDevice.getImageMemoryRequirements(mPerlinNoise)};
        vk::MemoryAllocateInfo allocateInfo{memReqs.size, vk_fn::getMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
        vkCreate([&](){ mPerlinNoiseMemory = mDevice.allocateMemory(allocateInfo); }, "allocate mPerlinNoise memory");
        //
        mDevice.bindImageMemory(mPerlinNoise, mPerlinNoiseMemory, 0);


        vk::ImageViewCreateInfo imageViewCreateInfo{
                {}, mPerlinNoise, vk::ImageViewType::e3D, vk::Format::eR32Sfloat, {},
                {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
        };
        vkCreate([&](){ mPerlinNoiseView = mDevice.createImageView(imageViewCreateInfo);}, "create perlin noise image view");

        vk::SamplerCreateInfo samplerCreateInfo{
                {}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                0.f, vk::True, 16.f, vk::False, {}, 0.f, {},
                {}, vk::False
        };
        vkCreate([&](){ mPerlinNoiseSampler = mDevice.createSampler(samplerCreateInfo);}, "perlin noise sampler view");
    }

    void volumeFog::copyDataTo3DPerlinNoise(vk::CommandBuffer& cmd) {
        mNoiseBuf = allocManager::build::bufSptr(m3DPerlinNoise.size() * sizeof (float), vk::BufferUsageFlagBits::eTransferSrc);
        mNoiseBuf->updateBuffer(m3DPerlinNoise);

        vk::ImageMemoryBarrier memoryBarrier{
                {}, {}, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, mPerlinNoise,
                {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
        };
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands,
                            {}, {}, {}, memoryBarrier);

        vk::BufferImageCopy region{
            0, 0, 0,
            {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {},
            {mNoiseSize, mNoiseSize, depth}
        };
        cmd.copyBufferToImage(mNoiseBuf->getBuffer(), mPerlinNoise, vk::ImageLayout::eTransferDstOptimal, region);

        memoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands,
                            {}, {}, {}, memoryBarrier);
    }


    void volumeFog::createFogPipeline() {
        mFogDescriptor.addDescriptorSetLayout({
                                                      vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                              });

        mFogPipelineLayout = mFogDescriptor.getPipelineLayout();
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mFogPipelineLayout, mRenderPass};
        pipelineCombined.depthStencilState.setDepthTestEnable(vk::False)
                .setDepthWriteEnable(vk::False);
        pipelineCombined.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
        pipelineCombined.addShader(ke_q::loadFile("v_fog.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_fog.spv"), vk::ShaderStageFlagBits::eFragment);

        pipelineCombined.updateState();
        mFogPipeline = pipelineCombined.createGraphicsPipeline();
    }

    void volumeFog::createFogDescriptor() {
        mFogDescriptor.increaseMaxSets()
                .createDescriptorPool()
                .createDescriptorSets()
                .addDescriptorSet({{
                                           vk::DescriptorImageInfo{mPerlinNoiseSampler, mPerlinNoiseView,vk::ImageLayout::eShaderReadOnlyOptimal},
                                   }}).update();
    }

    void  volumeFog::calculateGaussianWeights(int radius, float sigma) {
        weights.resize(radius + 1);
        float sum = 0.0f;

        for (int i = 0; i <= radius; i++) {
            weights[i] = exp(-(i * i) / (2 * sigma * sigma));
            sum += 2 * weights[i];
        }
        weights[0] += weights[0]; // 中心权重只计算一次
        sum -= weights[0]; // 减去重复添加的中心权重

        // 归一化权重
        for (int i = 0; i <= radius; i++) {
            weights[i] /= sum;
        }
    }

    void volumeFog::applyGaussianBlur3D(std::vector<float>& input, std::vector<float>& output, int width, int height, int depth_, const std::vector<float>& weights_, int radius) {
        int index = 0;
        for (int z = 0; z < depth_; z++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    float blurredValue = 0.0f;
                    float weightSum = 0.0f;

                    for (int dz = -radius; dz <= radius; dz++) {
                        for (int dy = -radius; dy <= radius; dy++) {
                            for (int dx = -radius; dx <= radius; dx++) {
                                int nx = x + dx;
                                int ny = y + dy;
                                int nz = z + dz;

                                // Check boundaries
                                if (nx >= 0 && nx < width && ny >= 0 && ny < height && nz >= 0 && nz < depth_) {
                                    int neighborIndex = (nz * height * width) + (ny * width) + nx;
                                    int distance = dz * dz + dy * dy + dx * dx;
                                    float weight = weights_[std::sqrt(distance)];

                                    blurredValue += input[neighborIndex] * weight;
                                    weightSum += weight;
                                }
                            }
                        }
                    }

                    output[index] = blurredValue / weightSum;
                    index++;
                }
            }
        }
    }

} // yic





































//void volumeFog::create3DPerlinNoise() {
//    FastNoiseLite noise;
//    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//    noise.SetFrequency(0.01f);
//
//    m3DPerlinNoise.resize(mNoiseSize * mNoiseSize * depth);
//
//    float amplitudeFactor = 0.5f;
//    float scale = 0.2f;
//
//    for(int index = 0, z = 0; z < depth; z++){
//        for(int y = 0; y < mNoiseSize; y++){
//            for(int x = 0; x < mNoiseSize; x++){
//                //float heightFactor = 1.f - (float)y / mNoiseSize;
//                m3DPerlinNoise[index] = (noise.GetNoise((float)x, (float)y, (float)z) + 0.8f) * (float )0.4;
//                //m3DPerlinNoise[index] *= heightFactor;
//                //m3DPerlinNoise[index] = noise.GetNoise((float)x / scale, (float)y / scale, (float)z / scale);
//
//                //m3DPerlinNoise[index] *= amplitudeFactor;
//
//                index ++;
//            }
//        }
//    }
//
//    mBlur3DPerlinNoise.resize(m3DPerlinNoise.size());
//    int radius = 3; float sigma = 1.f;
//
////        calculateGaussianWeights(radius, sigma);
////        applyGaussianBlur3D(m3DPerlinNoise, mBlur3DPerlinNoise, mNoiseSize, mNoiseSize, depth, weights, radius);
//}







