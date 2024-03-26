//
// Created by lenovo on 12/19/2023.
//

#include "vk_context.h"

namespace yic {

    vk_context::vk_context() = default;
    vk_context::~vk_context() = default;

    vk_context &vk_context::init() {
        assert(vk_init::get()->device() != VK_NULL_HANDLE && "must init the class init_context first!!! T^T");

        setup(vk_init::get());

        // transfer pool
        auto transferCmdPoolInfo = vk::CommandPoolCreateInfo().setQueueFamilyIndex(mGraphicsIndex)
                .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        mTransferPool = mDevice.createCommandPool(transferCmdPoolInfo);
        vkPmx::StagingBuffer::init(vk_init::get(), mTransferPool);

        return *this;
    }

    vk_context &vk_context::prepareEveryContext() {
        this            ->preSkyContext();
        this            ->prePmxContext();

        mPmxModel = mPmxContext->getPmxModel();

        return *this;
    }

    vk_context &vk_context::updateEveryFrame() {
        this            ->prepareFrame();
        mPmxContext     ->updateEveryFrame(vkCamera::get()->getViewMatrix(), vkCamera::get()->getProjMatrix(mExtent));
        mSkyboxContext  ->updateEveryFrame();

        return *this;
    }

    vk_context &vk_context::orRender() {
        mSkyboxContext  ->renderSkybox();

        return *this;
    }

    vk_context &vk_context::rasterization(const vk::CommandBuffer &cmd) {
        this            ->setViewport(cmd);

        mPmxContext     ->updateAnimation();
        mPmxContext     ->update();
        mPmxContext     ->draw(cmd);

        return *this;
    }

    ///-------------------------------------------------------------------------------------///
    ///-------------------------------------------------------------------------------------///
    ///                                   split up                                          ///
    ///-------------------------------------------------------------------------------------///
    ///-------------------------------------------------------------------------------------///


    vk_context &vk_context::prePmxContext() {
        std::vector<vkPmx::Input> inputModels{{R"(E:\Material\model\Nilou\Nilou.pmx)", {R"(E:\Material\vmd\mmd\8.vmd)"}}};
        mPmxContext = std::make_unique<vkPmx::pmx_context>(inputModels, vk_init::get(), mSwapchain.getImageCount(), mRenderPass, mCommandPool);

        return *this;
    }

    vk_context &vk_context::preSkyContext() {
        mSkyboxContext = std::make_unique<vkSkybox>(vk_init::get(), mSwapchain);

        return *this;
    }

} // yic













//// 备用


//vk_context &vk_context::createGraphicsPipeline() {
//    mDescriptor.addDescriptorSetLayout({
//                                               vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1,vk::ShaderStageFlagBits::eVertex},
//                                               vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eStorageBuffer, 1,vk::ShaderStageFlagBits::eVertex},
//                                       }).addDescriptorSetLayout({
//                                                                         vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
//                                                                 });
//
//    vk::PipelineLayoutCreateInfo createInfo{{}, mDescriptor.getDescriptorSetLayouts(), {}};
//    vkCreate([&](){ mPipelineLayout = mDevice.createPipelineLayout(createInfo); }, "create graphics pipeline layout");
//
//    graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mPipelineLayout, mRenderPass};
//    pipelineCombined.depthStencilState.depthTestEnable = true;
//    pipelineCombined.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
//    pipelineCombined.addShader(ke_q::loadFile("vertex.spv"), vk::ShaderStageFlagBits::eVertex);
//    pipelineCombined.addShader(ke_q::loadFile("fragment.spv"), vk::ShaderStageFlagBits::eFragment);
//    pipelineCombined.addBindingDescription({0, 7 * sizeof(float), vk::VertexInputRate::eVertex})
//            .addAttributeDescriptions({
//                                              {0, 0, vk::Format::eR32G32Sfloat, 0},
//                                              {1, 0, vk::Format::eR32G32B32Sfloat, 2 * sizeof(float)},
//                                              {2, 0, vk::Format::eR32G32Sfloat, 5 * sizeof(float)},
//                                      });
//    mGraphicsPipeline = pipelineCombined.createGraphicsPipeline();
//
//    return *this;
//}
//
//vk_context &vk_context::updateDescriptorSet() {
//    /// allocate model
//    mModelBuffer = std::make_unique<genericBufferManager>(sizeof (glm::mat4) * 1024, mScene->mMatrix.data(), vk::BufferUsageFlagBits::eStorageBuffer);
//    /// allocate camera
//    mCameraBuffer = std::make_unique<genericBufferManager>(sizeof (glm::mat4),vk::BufferUsageFlagBits::eUniformBuffer);
//
//    /// load texCoord
//    mImage = std::make_unique<vkImage>(texPath "2.png");
//
//    /// descriptor set
//    mDescriptor
//            .createDescriptorPool()
//            .createDescriptorSets()
//            .addDescriptorSet({
//                                      vk::DescriptorBufferInfo{mCameraBuffer->getBuffer(), 0, sizeof(glm::mat4)},
//                                      vk::DescriptorBufferInfo{mModelBuffer->getBuffer(), 0, mScene->mMatrix.size() * sizeof(glm::mat4)}
//                              })
//            .addDescriptorSet({
//                                      vk::DescriptorImageInfo{mImage->getSampler(), mImage->getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal},
//                              })
//            .update();
//
//    return *this;
//}
//
//vk_context &vk_context::prepare() {
//    // prepare buffer
//    mVertexBuffer = std::make_unique<genericBufferManager>(sizeof (float) * mMeshes->getMeshes().size()  , mMeshes->getMeshes().data(), vk::BufferUsageFlagBits::eVertexBuffer);
//
//    mBindBuffers.emplace_back(mVertexBuffer->getBuffer());
//    mOffsets.emplace_back(0);
//
//    return *this;
//}

//vk_context &vk_context::rasterization(const vk::CommandBuffer &cmd) {
//    //vkCamera::get()->updateCamera(mCameraBuffer.get(), mExtent);
//
//    setViewport(cmd);
//        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsPipeline);
//        cmd.bindVertexBuffers(0, 1, mBindBuffers.data(), mOffsets.data());
//        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, mDescriptor.getDescriptorSets(),nullptr);
//        cmd.draw(mMeshes->mSizes.find(meshTypes::eTriangle)->second, mScene->trianglePositions.size(), mMeshes->mOffsets.find(meshTypes::eTriangle)->second, 0);
//        cmd.draw(mMeshes->mSizes.find(meshTypes::eSquare)->second, mScene->squarePositions.size(), mMeshes->mOffsets.find(meshTypes::eSquare)->second, mScene->trianglePositions.size());
//        cmd.draw(mMeshes->mSizes.find(meshTypes::eStar)->second, mScene->starPositions.size(), mMeshes->mOffsets.find(meshTypes::eStar)->second, mScene->trianglePositions.size() + mScene->squarePositions.size());

//        auto pmxCmd = mPmxContext->getCmdBuf(mSwapchain.getActiveImageIndex());
//        cmd.executeCommands(1, &pmxCmd);
//
//    mPmxContext->updateAnimation();
//    mPmxContext->update();
//    mPmxContext->draw(cmd);
//
//    return *this;
//}