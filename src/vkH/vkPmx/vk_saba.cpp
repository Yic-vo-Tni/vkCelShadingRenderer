//
// Created by lenovo on 1/7/2024.
//

#include "vk_saba.h"

bool AppContext::Setup(yic::vk_init* vkInit) {
    m_vkInst = vkInit->getInstance();
    m_surface = vkInit->surfaceKhr();
    m_gpu = vkInit->physicalDevice();
    m_device = vkInit->device();
    mMmdDescriptor.init(m_device);

    m_resourceDir = saba::PathUtil::GetExecutablePath();
    m_resourceDir = saba::PathUtil::GetDirectoryName(m_resourceDir);
    m_resourceDir = saba::PathUtil::Combine(m_resourceDir, "resource");
    m_shaderDir = saba::PathUtil::Combine(m_resourceDir, "shader");
    m_mmdDir = saba::PathUtil::Combine(m_resourceDir, "mmd");

    // Get Memory Properties
    auto memProperties = m_gpu.getMemoryProperties();
    m_memProperties = memProperties;

    mvkContext.init();
    mvkContext.createSwapchain(yic::vk_init::get()->surfaceKhr(), yic::Window::get());
    mvkContext.createDepthBuffer();
    mvkContext.createRenderPass();
    mvkContext.createFrameBuffer();
    // Get Queue
    m_graphicsQueue = vkInit->graphicsQueue();
    m_graphicsQueueFamilyIndex = vkInit->getGraphicsFamilyIndices();

    m_imageCount = mvkContext.getSwapchain().getImageCount();

    m_commandPool = mvkContext.getCommandPool();

    // Create command pool (Transfer)
    auto transferCmdPoolInfo = vk::CommandPoolCreateInfo()
            .setQueueFamilyIndex(m_graphicsQueueFamilyIndex)
            .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    m_transferCommandPool = m_device.createCommandPool(transferCmdPoolInfo);

    m_depthFormat = mvkContext.getDepthFormat();
    m_colorFormat = mvkContext.getColorFormat();
    m_swapchain = mvkContext.getSwapchain().getSwapchain();
    m_swapchainImageResouces.resize(mvkContext.getSwapchain().getImageCount());
    for (size_t i = 0; i < mvkContext.getSwapchain().getImageCount(); i++)
    {
        m_swapchainImageResouces[i].m_image = mvkContext.getSwapchain().getImage(i);
        m_swapchainImageResouces[i].m_imageView = mvkContext.getSwapchain().getImageView(i);
        m_swapchainImageResouces[i].m_cmdBuffer = mvkContext.getCmdBuffer()[i];
    }
    m_imageIndex = 0;

    m_frameSyncDatas.resize(m_imageCount);
    for(int i = 0; i < m_imageCount; i++){
        m_frameSyncDatas[i].m_presentCompleteSemaphore = mvkContext.getSwapchain().getEntries()[i].readSemaphore;
        m_frameSyncDatas[i].m_renderCompleteSemaphore = mvkContext.getSwapchain().getEntries()[i].writtenSemaphore;
        m_frameSyncDatas[i].m_fence = mvkContext.getFences()[i];
    }


    m_renderPass = mvkContext.getRenderPass();

    for(int i = 0; i < mvkContext.getSwapchain().getImageCount(); i++){
        m_swapchainImageResouces[i].m_framebuffer = mvkContext.getFrameBuffers()[i];
    }


    mMmdDescriptor.addDescriptorSetLayout({
        std::make_pair(vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}, 1),
        std::make_pair(vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment}, 1),
        std::make_pair(vk::DescriptorSetLayoutBinding{2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}, 1)
                                          });

    m_mmdPipelineLayout = mMmdDescriptor.getPipelineLayout();
    yic::graphicsPipelineGeneratorCombined pipelineCombined{m_device, m_mmdPipelineLayout, m_renderPass};
    pipelineCombined.rasterizationState.setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthClampEnable(false).setRasterizerDiscardEnable(false)
            .setDepthBiasEnable(false);
    pipelineCombined.depthStencilState.setBack(vk::StencilOpState().setFailOp(vk::StencilOp::eKeep).setPassOp(vk::StencilOp::eKeep).setCompareOp(vk::CompareOp::eAlways));
    pipelineCombined.depthStencilState.front = pipelineCombined.depthStencilState.back;
    pipelineCombined.addShader(ke_q::loadFile("v_mmd.spv"), vk::ShaderStageFlagBits::eVertex);
    pipelineCombined.addShader(ke_q::loadFile("f_mmd.spv"), vk::ShaderStageFlagBits::eFragment);
    pipelineCombined.addBindingDescription({0, sizeof(Vertex), vk::VertexInputRate::eVertex});
    pipelineCombined.addAttributeDescriptions({
                                                      {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, m_position)},
                                                      {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, m_normal)},
                                                      {2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, m_uv)},
                                              });
    vk::PipelineColorBlendAttachmentState state{yic::graphicsPipelineState::makePipelineColorBlendAttachments({}, true,
                                                                                                              vk::BlendFactor::eSrcAlpha,vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
                                                                                                              vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha,vk::BlendOp::eAdd)};
    pipelineCombined.updateState();
    pipelineCombined.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
    m_mmdPipelines = pipelineCombined.createGraphicsPipeline();

    return true;
}

void AppContext::Destory()
{
    if (!m_device)
    {
        return;
    }

    m_device.waitIdle();
}