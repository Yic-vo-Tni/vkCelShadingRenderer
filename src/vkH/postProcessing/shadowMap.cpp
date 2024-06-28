//
// Created by lenovo on 4/24/2024.
//

#include "shadowMap.h"

namespace yic {

    shadowMap::shadowMap() = default;

    bool shadowMap::init(vk::Device device, vk::PhysicalDevice physicalDevice) {
        mDevice = device;
        mPhysicalDevice = physicalDevice;

        if (!getDepthFormat()){ vkError("failed to find the shadow map format");}
        createShadowMap();
        createShadowMapView();
        createShadowMapRenderPass();
        createShadowMapPipeline();
        createShadowMapFrameBuffer();
        createShadowMapCmd();
        updateImguiShadowMapSet();

        lightManager::get()->addDirectionalLight();
        //updateDirectionLightShadowMapSet();

        mConfiguration.texelSize = 1.f / (float )mExtent.width;
        mConfiguration.bias = 0.001f;
        mConfiguration.pcf_para = 1;

        mConfigBuf = allocManager::build::bufSptr(sizeof (shadowMapConfiguration), vk::BufferUsageFlagBits::eUniformBuffer);
        mConfigBuf->updateBuffer(mConfiguration);

        vkImguiManager::get()->addRenderToTest([&](){
            if (ImGui::CollapsingHeader("Shadow Map Config", ImGuiTreeNodeFlags_DefaultOpen)){
                ImGui::Checkbox("Enable Pcf", &mConfiguration.enablePcf);
                if (mConfiguration.enablePcf){
                    if(ImGui::SliderInt("Pcf parameter", &mConfiguration.pcf_para, 1, 5)){
                        mConfiguration.pcf = static_cast<int>(std::pow((mConfiguration.pcf_para * 2 + 1), 2));
                    }
                }

//                float step = (mConfiguration.bias < 0.01f) ? 0.001f : 0.01f;
//                ImGui::SliderFloat("Bias", &mConfiguration.bias, 0.f, 0.5f, "%.3f", step);
                if (ImGui::Button("Increase Bias")) {
                    if (mConfiguration.bias < 0.01f) {
                        mConfiguration.bias += 0.001f;
                    } else {
                        mConfiguration.bias += 0.01f;
                    }
                    mConfiguration.bias = (mConfiguration.bias > 1.0f) ? 1.0f : mConfiguration.bias;
                }
                ImGui::SameLine();
                if(ImGui::Button("DiCrease Bias")){
                    if (mConfiguration.bias <= 0.01f) {
                        mConfiguration.bias -= 0.001f;
                    } else {
                        mConfiguration.bias -= 0.01f;
                    }
                    mConfiguration.bias = (mConfiguration.bias > 1.0f) ? 1.0f : mConfiguration.bias;
                }
                ImGui::SameLine();
                ImGui::Text("Current Bias: %.3f", mConfiguration.bias);

                ImGui::SliderFloat("ShadowStrength", &mConfiguration.shadow, 0.1f, 1.f, "%.3f");
            }
        });

        return true;
    }

    vk::CommandBuffer& shadowMap::renderShadowMapBegin() {
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        std::vector<vk::ClearValue> clearValues{vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f}, vk::ClearDepthStencilValue{1.f, 0}};

        mShadowMapCmd.begin(beginInfo);

        vk::RenderPassBeginInfo renderPassBeginInfo{mShadowRenderPass, mShadowMapFrameBuf,
                                                        {{0, 0}, mExtent}, clearValues};


        mShadowMapCmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport{0.f, 0.f, static_cast<float>(mExtent.width), static_cast<float>(mExtent.height), 0.f, 1.f};
        mShadowMapCmd.setViewport(0, viewport);

        vk::Rect2D scissor{{0, 0}, {mExtent.width, mExtent.height}};
        mShadowMapCmd.setScissor(0, scissor);


        return mShadowMapCmd;
    }

    bool shadowMap::renderShadowMapEnd() {
        mShadowMapCmd.endRenderPass();
        mShadowMapCmd.end();

        return true;
    }



    bool shadowMap::createShadowMap() {
        if (mShadowMapView)
            mDevice.destroy(mShadowMapView);
        if (mShadowMap)
            mDevice.destroy(mShadowMap);
        if (mShadowMapMemory)
            mDevice.free(mShadowMapMemory);

        vk::ImageCreateInfo createInfo{{},
                                       vk::ImageType::e2D, mShadowMapFormat, vk::Extent3D{mExtent, 1},
                                       1, 1, vk::SampleCountFlagBits::e1, {},
                                       vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled};
        vkCreate([&](){ mShadowMap = mDevice.createImage(createInfo); }, "create depth image");

        ///
        vk::MemoryRequirements memReqs{mDevice.getImageMemoryRequirements(mShadowMap)};
        vk::MemoryAllocateInfo allocateInfo{memReqs.size, vk_fn::getMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
        vkCreate([&](){ mShadowMapMemory = mDevice.allocateMemory(allocateInfo); }, "allocate depth memory");
        //
        mDevice.bindImageMemory(mShadowMap, mShadowMapMemory, 0);

        if (mColorShadowMapView)
            mDevice.destroy(mColorShadowMapView);
        if (mColorShadowMap)
            mDevice.destroy(mColorShadowMap);
        if (mColorShadowMapMemory)
            mDevice.free(mColorShadowMapMemory);

        vk::ImageCreateInfo createInfo_c{{},
                                       vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm, vk::Extent3D{mExtent, 1},
                                       1, 1, vk::SampleCountFlagBits::e1, {},
                                       vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled};
        vkCreate([&](){ mColorShadowMap = mDevice.createImage(createInfo_c); }, "create depth image");

        ///
        vk::MemoryRequirements memReqs_c{mDevice.getImageMemoryRequirements(mColorShadowMap)};
        vk::MemoryAllocateInfo allocateInfo_c{memReqs_c.size, vk_fn::getMemoryType(mPhysicalDevice, memReqs_c.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
        vkCreate([&](){ mColorShadowMapMemory = mDevice.allocateMemory(allocateInfo_c); }, "allocate depth memory");
        //
        mDevice.bindImageMemory(mColorShadowMap, mColorShadowMapMemory, 0);

        return true;
    }

    bool shadowMap::createShadowMapView() {
        mShadowMapView = vk_fn::createImageView(mDevice, mShadowMap, mShadowMapFormat, vk::ImageAspectFlagBits::eDepth);
        //mShadowMapView = vk_fn::createImageView(mDevice, mShadowMap, mShadowMapFormat, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
        mColorShadowMapView = vk_fn::createImageView(mDevice, mColorShadowMap, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);

        return true;
    }

    bool shadowMap::createShadowMapRenderPass() {
        std::vector<vk::AttachmentDescription> attachmentDescription{{{},
                                                                             vk::Format::eR8G8B8A8Unorm, vk::SampleCountFlagBits::e1,
                                                                             vk::AttachmentLoadOp::eClear,
                                                                             vk::AttachmentStoreOp::eStore,
                                                                             vk::AttachmentLoadOp::eDontCare,
                                                                             vk::AttachmentStoreOp::eDontCare,
                                                                             vk::ImageLayout::eUndefined,
                                                                             vk::ImageLayout::eColorAttachmentOptimal},
                                                                     {{},
                                                                             mShadowMapFormat, vk::SampleCountFlagBits::e1,
                                                                             vk::AttachmentLoadOp::eClear,
                                                                             vk::AttachmentStoreOp::eStore,
                                                                             vk::AttachmentLoadOp::eDontCare,
                                                                             vk::AttachmentStoreOp::eDontCare,
                                                                             vk::ImageLayout::eUndefined,
                                                                             vk::ImageLayout::eShaderReadOnlyOptimal}};
        std::vector<vk::AttachmentReference> attachmentReference{{0, vk::ImageLayout::eColorAttachmentOptimal},
                                                                 {1, vk::ImageLayout::eDepthStencilAttachmentOptimal}};

        vk::SubpassDescription subpass{{}, vk::PipelineBindPoint::eGraphics, {}, attachmentReference[0], {}, &attachmentReference[1]};

        vk::RenderPassCreateInfo createInfo{{}, attachmentDescription, subpass, {}};
        vkCreate([&](){ mShadowRenderPass = mDevice.createRenderPass(createInfo); }, "create renderPass");

        return true;
    }

    bool shadowMap::createShadowMapFrameBuffer() {
        mDevice.destroy(mShadowMapFrameBuf);

        std::vector<vk::ImageView> mAttachments{mColorShadowMapView, mShadowMapView};
        vk::FramebufferCreateInfo createInfo{{}, mShadowRenderPass, mAttachments,
                                             mExtent.width, mExtent.height, 1};

        vkCreate([&]() { mShadowMapFrameBuf = mDevice.createFramebuffer(createInfo); }, "create shadow map framebuffer",0);

        return true;
    }


    bool shadowMap::createShadowMapCmd() {
        vkCreate([&](){ mShadowMapCmdPool = mDevice.createCommandPool(vk::CommandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer});}, "create shadow map pool");
        vkCreate([&](){ mShadowMapCmd = mDevice.allocateCommandBuffers(vk::CommandBufferAllocateInfo{mShadowMapCmdPool, vk::CommandBufferLevel::ePrimary, 1}).front();}, "create shadow map cmd");

        return true;
    }

    bool shadowMap::getDepthFormat() {
        auto feature{vk::FormatFeatureFlagBits::eDepthStencilAttachment};
        for (const auto &f: {vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint}) {
            vk::FormatProperties formatProperties{};
            mPhysicalDevice.getFormatProperties(f, &formatProperties);
            if ((formatProperties.optimalTilingFeatures & feature) == feature) {
                mShadowMapFormat = f;
                return true;
            }
        }

        return false;
    }

    bool shadowMap::createShadowMapPipeline() {
        mShadowMapDescriptor.addDescriptorSetLayout({
                                                            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                                    });

        mShadowMapPipelineLayout = mShadowMapDescriptor.getPipelineLayout();
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mShadowMapPipelineLayout, shadowMap::get()->getShadowMapRenderPass()};
        pipelineCombined.rasterizationState.setRasterizerDiscardEnable(vk::False);
        pipelineCombined.depthStencilState.setBack(vk::StencilOpState().setFailOp(vk::StencilOp::eKeep).setPassOp(vk::StencilOp::eKeep).setCompareOp(vk::CompareOp::eLess));
        pipelineCombined.depthStencilState.setDepthTestEnable(vk::True).setDepthWriteEnable(vk::True).setDepthBoundsTestEnable(vk::False).setStencilTestEnable(vk::False);
        pipelineCombined.depthStencilState.front = pipelineCombined.depthStencilState.back;
        pipelineCombined.addShader(ke_q::loadFile("v_shadowMap.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_shadowMap.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription({0, sizeof(yicVertex), vk::VertexInputRate::eVertex});
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(yicVertex, pos)},
                                                  });
        pipelineCombined.updateState();
        mShadowMapPipeline = pipelineCombined.createGraphicsPipeline();

        return true;
    }

    bool shadowMap::updateDirectionLightShadowMapSet() {
        mShadowMapDescriptor.addDescriptorSetLayout({
                                                            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                                    });
        // shadow map set
        mShadowMapDescriptor.increaseMaxSets( ).createDescriptorPool();

        mShadowMapDescriptor.pushBackDesSets();
        mShadowMapDescriptor.updateDescriptorSet({
                                                         vk::DescriptorBufferInfo{
                                                                 lightManager::get()->getDirectionLight()->getVpMatrixBuf()->getBuffer(),
                                                                 0,
                                                                 sizeof(glm::mat4 )
                                                         },
                                                 });

        return true;
    }

    bool shadowMap::updateImguiShadowMapSet() {
        mShadowSampler = mDevice.createSampler(
                {{}, vk::Filter::eLinear, vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear,
                 vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                 0.f, vk::False, 1.f,
                 vk::False, vk::CompareOp::eAlways,
                 0.f, 0.f,
                 vk::BorderColor::eIntOpaqueBlack, vk::False});

        // imgui set

        mImguiShadowMapDescriptor.addDescriptorSetLayout({
                                                                 vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                                         });

        mImguiShadowMapDescriptor.increaseMaxSets().createDescriptorPool();
        mImguiShadowMapDescriptor.pushBackDesSets()
                .updateDescriptorSet({
                                             vk::DescriptorImageInfo{
                                                     mShadowSampler,
                                                     mShadowMapView,
                                                     vk::ImageLayout::eShaderReadOnlyOptimal
                                             },
                                     });

        vkImguiManager::get()->addRenderToShadowMap([&](){
            ImGui::Image((ImTextureID) mImguiShadowMapDescriptor.getDescriptorSets().front(), {500, 500});
        });

        return true;
    }


} // yic