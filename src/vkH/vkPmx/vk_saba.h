//
// Created by lenovo on 1/7/2024.
//

#ifndef VKMMD_VK_SABA_H
#define VKMMD_VK_SABA_H

#include "yic_pch.h"

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/UnicodeUtil.h>
#include <Saba/Base/Time.h>
#include <Saba/Model/MMD/PMDModel.h>
#include <Saba/Model/MMD/PMXModel.h>
#include <Saba/Model/MMD/VMDFile.h>
#include <Saba/Model/MMD/VMDAnimation.h>
#include <Saba/Model/MMD/VMDCameraAnimation.h>

#include "resource_vulkan/mmd.vert.spv.h"
#include "resource_vulkan/mmd.frag.spv.h"

#include "miku/vk_fn.h"
#include "miku/context_vkinit.h"
#include "vkInit/vk_init.h"

#include "ke_q/file_operation.h"
#include "../vk_context.h"
#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"

struct AppContext;

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

// vertex

struct Vertex
{
    glm::vec3	m_position;
    glm::vec3	m_normal;
    glm::vec2	m_uv;
};

// MMD Shader uniform buffer

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

// Swap chain

struct SwapchainImageResource
{
    SwapchainImageResource() {}

    vk::Image		m_image;
    vk::ImageView	m_imageView;
    vk::Framebuffer	m_framebuffer;
    vk::CommandBuffer	m_cmdBuffer;

    void Clear(AppContext& appContext);
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
    static bool getStagingBuffer(AppContext& appContext, vk::DeviceSize memSize, StagingBuffer** outBuf);

    static vk::Device mDevice;
    static vk::PhysicalDevice mPhysicalDevice;
    static vk::CommandPool mTransferCommandPool;
    static vk::Queue mGraphicsQueue;
    static std::vector<std::unique_ptr<StagingBuffer>> mStagingBuffers;
};

struct AppContext : public yic::vk_context
{
    AppContext() {}

    std::string m_resourceDir;
    std::string	m_shaderDir;
    std::string	m_mmdDir;

    glm::mat4	m_viewMat;
    glm::mat4	m_projMat;
    int	m_screenWidth = 0;
    int	m_screenHeight = 0;

    glm::vec3	m_lightColor = glm::vec3(1, 1, 1);
    glm::vec3	m_lightDir = glm::vec3(-0.5f, -1.0f, -0.5f);

    float	m_elapsed = 0.0f;
    float	m_animTime = 0.0f;

    vk::Instance		m_vkInst;
    vk::SurfaceKHR		m_surface;
    vk::PhysicalDevice	m_gpu;
    vk::Device			m_device;

    vk::PhysicalDeviceMemoryProperties	m_memProperties;

    uint32_t	m_graphicsQueueFamilyIndex;

    vk::Format	m_colorFormat = vk::Format::eUndefined;
    vk::Format	m_depthFormat = vk::Format::eUndefined;

    // Sync Objects
    struct FrameSyncData
    {
        FrameSyncData() {}

        vk::Fence		m_fence;
        vk::Semaphore	m_presentCompleteSemaphore;
        vk::Semaphore	m_renderCompleteSemaphore;
    };
    std::vector<FrameSyncData>	m_frameSyncDatas;
    uint32_t					m_frameIndex = 0;

    // Buffer and Framebuffer
    vk::SwapchainKHR					m_swapchain;
    std::vector<SwapchainImageResource>	m_swapchainImageResouces;

    // Render Pass
    vk::RenderPass	m_renderPass;

    vk::DescriptorSetLayout	m_mmdDescriptorSetLayout;
    vk::PipelineLayout	m_mmdPipelineLayout;
    vk::Pipeline		m_mmdPipelines;
    vk::ShaderModule	m_mmdVSModule;
    vk::ShaderModule	m_mmdFSModule;

    yic::vk_context mvkContext;
    yic::vkDescriptor mMmdDescriptor;

    const uint32_t DefaultImageCount = 3;
    uint32_t	m_imageCount;
    uint32_t	m_imageIndex = 0;

    // Command Pool
    vk::CommandPool		m_commandPool;
    vk::CommandPool		m_transferCommandPool;

    // Queue
    vk::Queue			m_graphicsQueue;

    // Texture

    bool Setup(yic::vk_init* vkInit);
    void Destory();
};


#endif //VKMMD_VK_SABA_H
