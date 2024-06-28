//
// Created by lenovo on 4/29/2024.
//

#ifndef VKMMD_VOLUMEFOG_H
#define VKMMD_VOLUMEFOG_H

#include "FastNoiseLite.h"

#include "ke_q/vk_allocator.h"
#include "ke_q/file_operation.h"

#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"

namespace yic {

    class volumeFog {
    public:
        vkGet auto get = [](){ return Singleton<volumeFog>::get();};

        void init(vk::Device device, vk::PhysicalDevice physicalDevice);
        void create(vk::RenderPass renderPass);

        void draw(const vk::CommandBuffer& cmd);

        void copyDataTo3DPerlinNoise(vk::CommandBuffer& cmd);

        [[nodiscard]] inline auto& getPerlinNoiseImageView() const { return mPerlinNoiseView;}
    private:
        void create3DPerlinNoise();
        void create3dPerlinNoiseImage();

        void createFogPipeline();
        void createFogDescriptor();

        void calculateGaussianWeights(int radius, float sigma);
        void applyGaussianBlur3D(std::vector<float>& input, std::vector<float>& output, int width, int height, int depth_, const std::vector<float>& weights_, int radius);

    private:
        // init
        vk::Device mDevice{};
        vk::PhysicalDevice mPhysicalDevice{};
        vk::RenderPass mRenderPass{};

        uint32_t mNoiseSize = 520, depth = 10;
        std::vector<float> m3DPerlinNoise{};
        std::vector<float> mBlur3DPerlinNoise{};

        vk::Image mPerlinNoise{};
        vk::ImageView mPerlinNoiseView{};
        vk::Sampler mPerlinNoiseSampler{};
        vk::DeviceMemory mPerlinNoiseMemory{};

        vk::Pipeline mFogPipeline{};
        vk::PipelineLayout mFogPipelineLayout{};
        vkDescriptor mFogDescriptor{};

        allocManager::bufSptr mNoiseBuf{};

        std::vector<float> weights{};
    };

} // yic

#endif //VKMMD_VOLUMEFOG_H
