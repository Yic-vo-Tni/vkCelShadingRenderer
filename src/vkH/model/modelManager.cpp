//
// Created by lenovo on 4/22/2024.
//

#include "modelManager.h"

namespace yic {

    void modelManager::init(vk_init* vkInit,  uint32_t imageCount, const vk::RenderPass& renderPass) {
        mDevice = vkInit->device();
        mGraphicsQueue = vkInit->graphicsQueue();
        mImageCount = imageCount;
        mRenderPass = renderPass;

        load::init(vkInit->device(), vkInit->graphicsQueue());

        createCmdBuf();
        createDefaultPipeline();
    }

    bool modelManager::updateEveryFrame(glm::mat4 vp) {
        vkImguiManager::get()->getImguiTask(eObjectManager).clearCollapsingHeader();

        if (!mPath.empty()){
            loadModel(mPath);
            mPath.clear();
        }

        auto translateMat = glm::translate(glm::mat4 (1.f), glm::vec3 (0.f, 10.f, 0.f));
        mMvp.vpMatrix = vp * translateMat;

        for(auto it = mLoadModels.begin(); it != mLoadModels.end(); ){
            if ((*it)->mDelete){
                vk_init::get()->graphicsQueue().waitIdle();
                mDevice.waitIdle();
                it = mLoadModels.erase(it);
            } else{
                ++it;
            }
        }

        for(const auto& model : mLoadModels) {
            vkImguiManager::get()->addCollapsingHeaderTasksToObjManager(model->getPath(), [this, &model](){
                model->renderObjControls();
            });

            auto transform = model->getYicModel().transform;
            auto mvpMatrix = mMvp.vpMatrix * transform;

            mDirShadowMap.update(translateMat*transform);
            model->getLightMatrixBuf()->updateBuffer(mDirShadowMap.getMvpMatrix());

            for (const auto &obj: model->getYicModel().objects) {
                obj.mvpBuf->updateBuffer(mvpMatrix);
            }
        }


        return true;
    }

    void modelManager::loadModel(const std::string& path) {
        auto m = std::make_shared<load>(path);

        mLoadModels.emplace_back(m);
    }

    void modelManager::draw(const vk::CommandBuffer& cmd) {
        for(const auto& model : mLoadModels) {
            auto set = 0;
            for (const auto &obj: model->getYicModel().objects) {
                for (const auto &subMesh: obj.subMeshes) {
                    std::vector<vk::DeviceSize> offsets = {0};
                    std::vector<vk::Buffer> mBuffers{subMesh.vertBuf->getBuffer()};
                    cmd.bindVertexBuffers(0, mBuffers, offsets);
                    cmd.bindIndexBuffer(subMesh.indexBuf->getBuffer(), 0, vk::IndexType::eUint32);

                    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mDefaultPipeline);
                    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mDefaultPipeLayout, 0,
                                           model->getDescriptors()[set], nullptr);

                    cmd.drawIndexed(subMesh.indices.size(), 1, 0, 0, 0);

                    set++;
                }
            }
        }

    }

    void modelManager::drawShadowMap(const vk::CommandBuffer &cmd) {
        auto translateMat = glm::translate(glm::mat4 (1.f), glm::vec3 (0.f, 10.f, 0.f));
        for(const auto& model : mLoadModels) {
            for (const auto &obj: model->getYicModel().objects) {
                for (const auto &subMesh: obj.subMeshes) {
                    std::vector<vk::DeviceSize> offsets = {0};
                    std::vector<vk::Buffer> mBuffers{subMesh.vertBuf->getBuffer()};
                    cmd.bindVertexBuffers(0, mBuffers, offsets);
                    cmd.bindIndexBuffer(subMesh.indexBuf->getBuffer(), 0, vk::IndexType::eUint32);

                    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowMap::get()->getShadowMapPipeline());
                    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowMap::get()->getShadowMapPipelineLayout(), 0,
                                           mDirShadowMap.getSet().getDescriptorSets(), nullptr);

                    cmd.drawIndexed(subMesh.indices.size(), 1, 0, 0, 0);
                }
            }
        }
    }



    bool modelManager::createCmdBuf() {
        mCmdBufs.resize(mImageCount);

        mCmdPool = mDevice.createCommandPool(vk::CommandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

        vk::CommandBufferAllocateInfo allocateInfo{mCmdPool, vk::CommandBufferLevel::eSecondary, mImageCount};
        mCmdBufs = mDevice.allocateCommandBuffers(allocateInfo);

        return true;
    }

    bool modelManager::createDefaultPipeline() {
        mDescriptor.addDescriptorSetLayout({
                                                      vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                                      vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                                      vk::DescriptorSetLayoutBinding{2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                                      vk::DescriptorSetLayoutBinding{3, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                                      vk::DescriptorSetLayoutBinding{4, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
                                              });

        mDefaultPipeLayout = mDescriptor.getPipelineLayout();
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mDefaultPipeLayout, mRenderPass};
        pipelineCombined.rasterizationState.setFrontFace(vk::FrontFace::eCounterClockwise)
                .setDepthClampEnable(false).setRasterizerDiscardEnable(false)
                .setDepthBiasEnable(false);
        pipelineCombined.depthStencilState.setBack(vk::StencilOpState().setFailOp(vk::StencilOp::eKeep).setPassOp(vk::StencilOp::eKeep).setCompareOp(vk::CompareOp::eAlways));
        pipelineCombined.depthStencilState.front = pipelineCombined.depthStencilState.back;
        pipelineCombined.addShader(ke_q::loadFile("v_basic.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_basic.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription({0, sizeof(yicVertex), vk::VertexInputRate::eVertex});
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(yicVertex, pos)},
                                                          {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(yicVertex, nor)},
                                                          {2, 0, vk::Format::eR32G32Sfloat, offsetof(yicVertex, uv)},
                                                  });

        pipelineCombined.updateState();
        pipelineCombined.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
        mDefaultPipeline = pipelineCombined.createGraphicsPipeline();

        return true;
    }



} // yic