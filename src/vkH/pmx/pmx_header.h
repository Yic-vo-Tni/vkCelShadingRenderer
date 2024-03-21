//
// Created by lenovo on 1/13/2024.
//

#ifndef VKMMD_PMX_HEADER_H
#define VKMMD_PMX_HEADER_H

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/UnicodeUtil.h>
#include <Saba/Base/Time.h>
#include <Saba/Model/MMD/PMDModel.h>
#include <Saba/Model/MMD/PMXModel.h>
#include <Saba/Model/MMD/VMDFile.h>
#include <Saba/Model/MMD/VMDAnimation.h>
#include <Saba/Model/MMD/VMDCameraAnimation.h>

#include "miku/vk_fn.h"
#include "ke_q/file_operation.h"

#include "centralizedTaskSystem/vkCommon.h"
#include "niLou/vkDescriptor.h"
#include "niLou/vkPipeline.h"

#include "vkInit/vk_init.h"

namespace vkPmx {

    struct Input
    {
        std::string					m_modelPath;
        std::vector<std::string>	m_vmdPaths;
    };

    namespace
    {

        void SetImageLayout(vk::CommandBuffer cmdBuf, vk::Image image, vk::ImageAspectFlags aspectMask,
                            vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout,
                            vk::ImageSubresourceRange subresourceRange
        )
        {
            auto imageMemoryBarrier = vk::ImageMemoryBarrier()
                    .setOldLayout(oldImageLayout)
                    .setNewLayout(newImageLayout)
                    .setImage(image)
                    .setSubresourceRange(subresourceRange);
            switch (oldImageLayout)
            {
                case vk::ImageLayout::eUndefined:
                    imageMemoryBarrier.setSrcAccessMask(vk::AccessFlags());
                    break;
                case vk::ImageLayout::eTransferDstOptimal:
                    imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
                    break;
                case vk::ImageLayout::ePreinitialized:
                    imageMemoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eHostWrite);
                    break;
                default:
                    break;
            }

            switch (newImageLayout)
            {
                case vk::ImageLayout::eShaderReadOnlyOptimal:
                    imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
                    break;
                case vk::ImageLayout::eTransferSrcOptimal:
                    imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
                    break;
                case vk::ImageLayout::eTransferDstOptimal:
                    imageMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
                    break;
                default:
                    break;
            }

            vk::PipelineStageFlags srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
            vk::PipelineStageFlags destStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
            if (oldImageLayout == vk::ImageLayout::eUndefined &&
                newImageLayout == vk::ImageLayout::eTransferDstOptimal)
            {
                srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
                destStageFlags = vk::PipelineStageFlagBits::eTransfer;
            }
            else if (oldImageLayout == vk::ImageLayout::eTransferDstOptimal &&
                     newImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
            {
                srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
                destStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
            }

            cmdBuf.pipelineBarrier(
                    srcStageFlags,
                    destStageFlags,
                    vk::DependencyFlags(),
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier
            );
        }
    }

    struct Vertex
    {
        glm::vec3	m_position;
        glm::vec3	m_normal;
        glm::vec2	m_uv;

        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv)
            : m_position(position), m_normal(normal), m_uv(uv){}
    };

    struct MMDVertxShaderUB
    {
        glm::mat4	m_wv;
        glm::mat4	m_wvp;
    };

    struct MMDFragmentShaderUB
    {
        glm::vec3	m_diffuse;
        float		m_alpha;
        glm::vec3	m_ambient;
        float		m_dummy1;
        glm::vec3	m_specular;
        float		m_specularPower;
        glm::vec3	m_lightColor;
        float		m_dummy2;
        glm::vec3	m_lightDir;
        float		m_dummy3;

        glm::vec4	m_texMulFactor;
        glm::vec4	m_texAddFactor;

        glm::ivec4	m_textureModes;
    };

    struct StagingBuffer
    {
        StagingBuffer() = default;

        vk::DeviceMemory	m_memory;
        vk::Buffer			m_buffer;

        vk::DeviceSize		m_memorySize = 0;

        vk::CommandBuffer	m_copyCommand;
        vk::Fence			m_transferCompleteFence;
        vk::Semaphore		m_transferCompleteSemaphore;

        vk::Semaphore		m_waitSemaphore;

        static bool init(yic::vk_init* vkInit, const vk::CommandPool& transferPool);
        bool Setup(vk::DeviceSize size);
        void Clear();
        void Wait();
        bool CopyBuffer(vk::Buffer destBuffer, vk::DeviceSize size);
        bool CopyImage(
                vk::Image destImage,
                vk::ImageLayout imageLayout,
                uint32_t regionCount,
                vk::BufferImageCopy* regions
        );
        static bool getStagingBuffer(vk::DeviceSize memSize, StagingBuffer** outBuf);

        static vk::Device mDevice;
        static vk::PhysicalDevice mPhysicalDevice;
        static vk::CommandPool mTransferCommandPool;
        static vk::Queue mGraphicsQueue;
        static std::vector<std::unique_ptr<StagingBuffer>> mStagingBuffers;
    };

} // vkPmx

#endif //VKMMD_PMX_HEADER_H
