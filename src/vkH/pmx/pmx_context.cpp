//
// Created by lenovo on 1/13/2024.
//

#include "pmx_context.h"


namespace vkPmx {
    
    pmx_context::pmx_context(const std::vector<Input>& inputModels, yic::vk_init *vkInit, uint32_t imageCount, const vk::RenderPass &renderPass
                             ) : mvkInit{vkInit}, mImageCount(imageCount), mRenderPass(renderPass){
        mDevice = vkInit->device();
        mPhysicalDevice = vkInit->physicalDevice();
        if (!inputModels[0].m_vmdPaths.empty()) { mLoadAnim = true; }

        mResDir = saba::PathUtil::GetExecutablePath();
        mResDir = saba::PathUtil::GetDirectoryName(mResDir);
        mResDir = saba::PathUtil::Combine(mResDir, "resource");
        mShaderDir = saba::PathUtil::Combine(mResDir, "shader");
        mMmdDir = saba::PathUtil::Combine(mResDir, "mmd");

        loadPmxInputModels(inputModels);

        mSaveTime = saba::GetTime();

        mMaterialCount = mMmdModel->GetMaterialCount();
        //mMmdMaterials.resize(mMaterialCount);

        //subMesh
        mSubMesh.resize(mMmdModel->GetSubMeshCount());
        for(uint32_t i = 0; i < mSubMesh.size(); i++){
            mSubMesh[i].subMesh = mMmdModel->GetSubMeshes()[i];
        }

        prepareEveryThing();
    }

    pmx_context::~pmx_context() {
        StagingBuffer::clear();
        vkImguiManager::get()->getImguiTask(eToolbar).clear(mSliderAnim);
    }

    pmx_context &pmx_context::prepareEveryThing() {
        createCommandBuffer();
        createPipeline();
        createShadowPipeline();
        createVertexBuffer();
        createMaterialsTex();
        createDescriptorSet();

        return *this;
    }

    pmx_context &pmx_context::updateEveryFrame(const glm::mat4& view, const glm::mat4& proj){
        double time = saba::GetTime();
            double elapsed = time - mSaveTime;
            if (elapsed > 1.f / 30.f) {
                elapsed = 1.f / 30.f;
            }
            mSaveTime = time;
            mElapsed = float(elapsed);
        if (mPlayAnim){
            mAnimTime += float(elapsed);
        }

        mView = view;
        mProj = proj;

        mDirectionLightSpaceMatrixBuf->updateBuffer(lightManager::getDirectionLight()->getLightSpaceMatrix());

        return *this;
    }

    pmx_context &pmx_context::createPipeline() {
        mMmdDescriptor.addDescriptorSetLayout({
           vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
           vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
           vk::DescriptorSetLayoutBinding{2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
           vk::DescriptorSetLayoutBinding{3, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
           vk::DescriptorSetLayoutBinding{4, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
           vk::DescriptorSetLayoutBinding{5, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
           vk::DescriptorSetLayoutBinding{6, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
      //     vk::DescriptorSetLayoutBinding{7, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
        })
        .addDescriptorSetLayout({
            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
        })
        ;
        mImguiTexDescriptor.addDescriptorSetLayout({
            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}
        });

        mPipelineLayout = mMmdDescriptor.getPipelineLayout();
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mPipelineLayout, mRenderPass};
        pipelineCombined.rasterizationState.setFrontFace(vk::FrontFace::eCounterClockwise)
                .setDepthClampEnable(false).setRasterizerDiscardEnable(false)
                .setDepthBiasEnable(false);
        pipelineCombined.depthStencilState.setBack(vk::StencilOpState().setFailOp(vk::StencilOp::eKeep).setPassOp(vk::StencilOp::eKeep).setCompareOp(vk::CompareOp::eAlways));
        pipelineCombined.depthStencilState.front = pipelineCombined.depthStencilState.back;
        //pipelineCombined.depthStencilState.setMinDepthBounds(0.f).setMaxDepthBounds(1.f);
        pipelineCombined.addShader(ke_q::loadFile("v_mmd.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_mmd.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription({0, sizeof(yicVertex), vk::VertexInputRate::eVertex});
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(yicVertex, pos)},
                                                          {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(yicVertex, nor)},
                                                          {2, 0, vk::Format::eR32G32Sfloat, offsetof(yicVertex, uv)},
                                                  });

        pipelineCombined.updateState();
        pipelineCombined.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
        mPipeline = pipelineCombined.createGraphicsPipeline();

        return *this;
    }

    pmx_context &pmx_context::createShadowPipeline() {
        mGroundShadowDescriptor.addDescriptorSetLayout({
            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
            vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
            });
        mGroundShadowPipelineLayout = mGroundShadowDescriptor.getPipelineLayout();
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mGroundShadowPipelineLayout, mRenderPass};
        pipelineCombined.rasterizationState.setFrontFace(vk::FrontFace::eCounterClockwise)
                .setDepthClampEnable(false).setRasterizerDiscardEnable(false)
                .setDepthBiasEnable(false);
        pipelineCombined.depthStencilState.setBack(vk::StencilOpState().setFailOp(vk::StencilOp::eKeep).setPassOp(vk::StencilOp::eKeep).setCompareOp(vk::CompareOp::eAlways));
        pipelineCombined.depthStencilState.front = pipelineCombined.depthStencilState.back;
        std::vector<vk::DynamicState> dynamicState = {
                vk::DynamicState::eDepthBias,
                vk::DynamicState::eStencilReference,
                vk::DynamicState::eStencilCompareMask,
                vk::DynamicState::eStencilWriteMask
        };
        pipelineCombined.addDyState(dynamicState);
        pipelineCombined.addShader(ke_q::loadFile("v_mmd_ground_shadow.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_mmd_ground_shadow.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription({0, sizeof(yicVertex), vk::VertexInputRate::eVertex});
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(yicVertex, pos)},
                                                  });
        pipelineCombined.updateState();
        pipelineCombined.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
        yic::graphicsPipelineGeneratorCombined::makePipelineColorBlendAttachments({}, false);
        mGroundShadowPipeline = pipelineCombined.createGraphicsPipeline();

        return *this;
    }

    pmx_context &pmx_context::createVertexBuffer() {
        /// vertex buffer
        auto vertBufMemSize = uint32_t (sizeof (yicVertex)) * mMmdModel->GetVertexCount();
        mModelResource.mVertexBuffer = allocManager::build::bufAccelAddressUptr(vertBufMemSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst |
                                                                                                                vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

        /// index buffer
        auto indexMemSize = uint32_t (mMmdModel->GetIndexCount() * mMmdModel->GetIndexElementSize());
        mModelResource.mIndexBuffer = allocManager::build::bufAccelAddressUptr(vertBufMemSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst |
                                                                                                              vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
        //
        StagingBuffer* indicesStagingBuffer;
        StagingBuffer::getStagingBuffer(indexMemSize, &indicesStagingBuffer);
        
        void* mapMem;
        if ( mDevice.mapMemory(indicesStagingBuffer->m_memory, 0, indexMemSize, vk::MemoryMapFlagBits(0), &mapMem) != vk::Result::eSuccess ){
            vkError("failed to map memory");
        }
        memcpy(mapMem, mMmdModel->GetIndices(), indexMemSize);
        mDevice.unmapMemory(indicesStagingBuffer->m_memory);

        indicesStagingBuffer->CopyBuffer(mModelResource.mIndexBuffer->getBuffer(), indexMemSize);
        indicesStagingBuffer->Wait();
        
        /// find index element size
        switch (mMmdModel->GetIndexElementSize()) {
            case 2:
                mModelResource.mIndexType = vk::IndexType::eUint16;
                mPmxModel.indexType = vk::IndexType::eUint16;
                break;
            case 4:
                mModelResource.mIndexType = vk::IndexType::eUint32;
                mPmxModel.indexType = vk::IndexType::eUint32;
                break;
            default:
                vkError("unknown index size {0}", mMmdModel->GetIndexElementSize());
                break;
        }

        mPmxModel.mVertexBuffer.buffer = mModelResource.mVertexBuffer->getBuffer();
        mPmxModel.mIndexBuffer.buffer = mModelResource.mIndexBuffer->getBuffer();
        mPmxModel.indices = mMmdModel->GetIndexCount();
        mPmxModel.vertices = mMmdModel->GetVertexCount();


        return *this;
    }

    pmx_context &pmx_context::createMaterialsTex() {
        std::cout << mMmdModel->GetSubMeshes() << std::endl;
        std::cout << mMmdModel->GetSubMeshCount() << std::endl;

        auto cmd = createTempCmdBuf();
        for(size_t i = 0; i < mMaterialCount; i++){
            auto& path = mMmdModel->GetMaterials()[i].m_texture;
            if (mSptrMaterials.find(path) == mSptrMaterials.end()){
                mSptrMaterials[path] = std::make_shared<pmx_material>(path, cmd);
            }

            if (mSptrMaterials[path]){
                mMaterials.emplace_back(mSptrMaterials[path]);
            }
        }

        mFaceBlush = std::make_shared<vkImage>(texPath "pmx/face_diffuse(1).png");
        mFaceBlush->configureImageForRender(cmd);
        // ramp
        //mRamp = std::make_shared<vkImage>();

        submitTempCmdBuf(cmd);


        for(size_t i = 0; i < mMaterialCount; i++) {
            const auto &mmdMaterial = mMmdModel->GetMaterials()[i];

            auto& mat = mSubMesh[i].materialInfo;

            mat.m_alpha = mmdMaterial.m_alpha;
            mat.m_diffuse = mmdMaterial.m_diffuse;
            mat.m_ambient = mmdMaterial.m_ambient;
            mat.m_specular = mmdMaterial.m_specular;
            mat.m_specularPower = mmdMaterial.m_specularPower;

            if (!mmdMaterial.m_texture.empty()) {
                if (!mMaterials[i]->getTexture()->hasAlpha()) {
                    mat.m_textureModes.x = 1;
                } else {
                    mat.m_textureModes.x = 2;
                }
                mat.m_texAddFactor = mmdMaterial.m_textureAddFactor;
            } else {
                mat.m_textureModes.x = 0;
            }

            mat.m_lightColor = mLightColor;
            glm::vec3 lightDir = mLightDir;
            auto viewMaterial = glm::mat3(mView);
            lightDir = viewMaterial * lightDir;
            mat.m_lightDir = lightDir;

            mSubMesh[i].mmdTextureEmpty = mmdMaterial.m_texture.empty();
        }

        return *this;
    }

    pmx_context &pmx_context::createDescriptorSet() {
        for(auto& subMesh : mSubMesh){
            subMesh.mUniformBuf = allocManager::build::bufUptr(sizeof (MMDVertxShaderUB) + sizeof (MMDFragmentShaderUB), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst);
        }

        mDirectionLightSpaceMatrixBuf = allocManager::build::bufSptr(sizeof (glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer);

        //descriptor
        mMmdDescriptor.increaseMaxSets(mMaterialCount * 2)
                .createDescriptorPool();

        for(size_t i = 0; i < mMaterialCount; i++){
            mMmdDescriptor.pushBackDesSets();
            mMmdDescriptor.updateDescriptorSet({
                                                       vk::DescriptorBufferInfo{
                                                               mSubMesh[i].mUniformBuf->getBuffer(),
                                                               0,
                                                               sizeof(MMDVertxShaderUB)
                                                       },
                                                       vk::DescriptorBufferInfo{
                                                           mSubMesh[i].mUniformBuf->getBuffer(),
                                                           sizeof (MMDVertxShaderUB),
                                                           sizeof (MMDFragmentShaderUB)
                                                       },
                                                       vk::DescriptorImageInfo{
                                                           mMaterials[i]->getTexture()->getSampler(),
                                                           mMaterials[i]->getTexture()->getImageView(),
                                                           vk::ImageLayout::eShaderReadOnlyOptimal
                                                       },
                                                       vk::DescriptorBufferInfo{
                                                           mMaterials[i]->getEffUnifBuf()->getBuffer(),
                                                           0,
                                                           sizeof(MMDFragEffect)
                                                       },
                                                       vk::DescriptorImageInfo{
                                                               mMaterials[i]->getTexture()->getSampler(),
                                                               shadowMap::get()->getShadowMapView(),
                                                               vk::ImageLayout::eShaderReadOnlyOptimal
                                                       },
                                                       vk::DescriptorBufferInfo{
                                                               mDirectionLightSpaceMatrixBuf->getBuffer(),
                                                               0,
                                                               sizeof(glm::mat4 )
                                                       },
                                                       vk::DescriptorBufferInfo{
                                                               shadowMap::get()->getConfigurationBuf()->getBuffer(),
                                                               0,
                                                               sizeof (shadowMapConfiguration)
                                                       },
                                               });
        }

        mMmdDescriptor.pushBackDesSets(1);
        mMmdDescriptor.updateDescriptorSet({
                                                   vk::DescriptorImageInfo{
                                                        mFaceBlush->getSampler(),
                                                        mFaceBlush->getImageView(),
                                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                                   }
                                           }, 1);
        //imgui tex
        mImguiTexDescriptor.increaseMaxSets(mMaterialCount - 1)
                        .createDescriptorPool();

        for(size_t i = 0; i < mMaterialCount; i++){
            mImguiTexDescriptor.pushBackDesSets()
                            .updateDescriptorSet({
                                vk::DescriptorImageInfo{
                                        mMaterials[i]->getTexture()->getSampler(),
                                        mMaterials[i]->getTexture()->getImageView(),
                                    vk::ImageLayout::eShaderReadOnlyOptimal
                                }
                            });
        }

        return *this;
    }

    pmx_context &pmx_context::createCommandBuffer() {
        mCmdBuffers.resize(mImageCount);

        mCommandPool = mDevice.createCommandPool(vk::CommandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::eSecondary, mImageCount};
        mCmdBuffers = mDevice.allocateCommandBuffers(allocateInfo);

        return *this;
    }

    pmx_context &pmx_context::updateAnimation() {
        if (mPlayAnim){
            mMmdModel->BeginAnimation();
            mMmdModel->UpdateAllAnimation(mVmdAnimation.get(), mAnimTime * 30.f, mElapsed);
            mMmdModel->EndAnimation();
        }

        if (!vkImguiManager::get()->getImguiTask(eToolbar).findTask(mSliderAnim) && mLoadAnim){
            mSliderAnim = vkImguiManager::get()->addRenderToToolbar([this](){
                ImGui::SameLine();

                if (ImGui::Button(mPlayAnim ? "Pause" : "Play", ImVec2(100, 20))) {
                    mPlayAnim = !mPlayAnim;
                }
                ImGui::SameLine();
                ImGui::Text("Animation Control");

                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space))){
                    mPlayAnim = !mPlayAnim;
                }

                if (ImGui::SliderFloat("frame", &mAnimTime, 0, 100) ) {
                        mMmdModel->BeginAnimation();
                        mMmdModel->UpdateAllAnimation(mVmdAnimation.get(),
                                                      mAnimTime * 30.f,
                                                      mElapsed);
                        mMmdModel->EndAnimation();
                    ImGui::SameLine();
                    ImGui::Text("current frame", 0, 100);
                }


                auto& eff = mMMDFragEff;
                if (ImGui::CollapsingHeader("Effects Controls", ImGuiTreeNodeFlags_DefaultOpen)) {

                    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll)) {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{15.f, 8.f});
                        ImGui::Dummy({0.f, 25.f});
                        if (ImGui::BeginTabItem("   Blur")) {
                            ImGui::SliderFloat("Blur Radius", &eff.blurRadius, 0.0f, 50.0f, "%.1f");
                            ImGui::SliderFloat("Blur Strength", &eff.blurStrength, 0.0f, 1.0f, "%.3f");
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("   Color")) {
                            ImGui::SliderFloat("Saturation Factor", &eff.saturationFactor, 0.0f, 10.f, "%.2f");
                            if (ImGui::ColorPicker4("Hue Color", color, ImGuiColorEditFlags_DisplayHSV | //ImGuiColorEditFlags_NoInputs  |
                                                                        ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel |
                                                                        ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)) {
                                float hue, saturation, value;
                                ImGui::ColorConvertRGBtoHSV(color[0], color[1], color[2], hue, saturation, value);

                                eff.hueAngle = float (hue * 2.f * M_PI);
                                eff.brightness = value;
                            }
                            if(ImGui::Checkbox("Enable Hdr", &hdrJudge)){
                                eff.brightness = 1.f;
                                hdr = 1.f;
                            };
                            if (hdrJudge){
                                if (ImGui::SliderFloat("HDR", &hdr, 1.f, 10.f)){
                                    eff.brightness = hdr;
                                }
                            }

                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("   DarkPart")){
                            ImGui::SliderFloat("Dart strength", &eff.darkPartStrength, 0.f, 1.f);

                            ImGui::SliderInt("Hard Dart or Smooth Dart", &eff.stepOrSmoothStep, 0, 1, "%d");

                            ImGui::SliderInt("Nums of steps", &eff.nums, 1, 10);
                            ImGui::SliderFloat("Base value", &eff.base, 0.f, 1.f);

                            if (eff.stepOrSmoothStep == 1) {
                                ImGui::SliderFloat("Upper Base value", &eff.baseUpper, 0.f, 1.f);
                                ImGui::SliderFloat("Range", &eff.range, 0.01f, 0.5f);
                            }
                            if (eff.stepOrSmoothStep == 0) {
                                ImGui::SliderFloat("Step size", &eff.stepSize, 0.01f, 0.1f);
                            }


                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("   Specular")){
                            ImGui::SliderFloat("Max Specular", &eff.maxSpecular, 0.0f, 1.0f, "%.3f");
                            ImGui::SliderFloat("Specular Color Strength", &eff.specularColorStrength, 0.0f, 1.0f, "%.3f");
                            ImGui::SliderFloat("Roughness Coefficient", &eff.roughnessCoefficient, 0.0f, 0.01f, "%.3f");
                            ImGui::SliderFloat("Specular Power", &eff.specularPow, 0.0f, 10.0f, "%.3f");

                            ImGui::EndTabItem();
                        }
                        ImGui::PopStyleVar();
                        ImGui::EndTabBar();
                    }
                }



            });
        }

        return *this;
    }

    pmx_context &pmx_context::update() {
        size_t vertexCount = mMmdModel->GetVertexCount();
        mMmdModel->Update();
        auto position{mMmdModel->GetUpdatePositions()};
        auto normal(mMmdModel->GetUpdateNormals());
        auto uv{mMmdModel->GetUpdateUVs()};

        /// update vertices buffer
        auto memSize{vk::DeviceSize{sizeof (yicVertex) * vertexCount}};
        StagingBuffer* vertBuf;
        StagingBuffer::getStagingBuffer(memSize, &vertBuf);
        void* vertStagingBufMem;
        if(mDevice.mapMemory(vertBuf->m_memory, 0, memSize, vk::MemoryMapFlags(), &vertStagingBufMem) != vk::Result::eSuccess){
            vkError("failed to map memory vert stag");
        }

        auto vertex = static_cast<yicVertex*>(vertStagingBufMem);
        for(size_t i = 0; i < vertexCount; i++){
            vertex[i] = yicVertex(position[i], normal[i], uv[i]);

            min = glm::min(position[i], min);
            max = glm::max(position[i], max);
        }
        mCenter = (min + max) / 2.f;
        mDevice.unmapMemory(vertBuf->m_memory);
        vertBuf->CopyBuffer(mModelResource.mVertexBuffer->getBuffer(), memSize);
        //
        auto indices = static_cast<const uint16_t*>(mMmdModel->GetIndices());
        bool ray = false, check = false;

        for(size_t i = 0; i < mMaterialCount; i++) {
            auto uniformSize = mSubMesh[i].mUniformBuf->getDeviceSize();
            StagingBuffer *uniformBuf;
            StagingBuffer::getStagingBuffer(uniformSize, &uniformBuf);
            uint8_t *uniformBufPtr;
            if (mDevice.mapMemory(uniformBuf->m_memory, 0, uniformSize, vk::MemoryMapFlags(),
                                  (void **) &uniformBufPtr) != vk::Result::eSuccess) {
                vkError("failed to map mem uniform stag");
            }
            // model unif
            auto mmdVertUnifBuf = reinterpret_cast<MMDVertxShaderUB*>(uniformBufPtr);

            auto translateToOrigin = glm::translate(glm::mat4 (1.f), -mSubMesh[i].center);
            auto scaleMatrix = glm::scale(glm::mat4 (1.f), mSubMesh[i].scale);
            glm::mat4 translateBlack = glm::translate(glm::mat4 (1.f), mSubMesh[i].center);
            glm::mat4 scale = translateBlack * scaleMatrix * translateToOrigin;

            auto modelTransform = modelTransformManager::get()->getModelMatrix() * modelTransformManager::get()->getScaleMatrix();
            auto finalMatrix = modelTransformManager::get()->getModelMatrix() * modelTransformManager::get()->getScaleMatrix() * glm::translate(glm::mat4 (1.f), mSubMesh[i].transform) * scale;
            mmdVertUnifBuf->m_wv = mView * finalMatrix;
            mmdVertUnifBuf->m_wvp = mProj * mView * finalMatrix;

            mDirShadowMap.update(modelTransform);

            /// AABB
            auto& mesh = mSubMesh[i];
            auto& m = mSubMesh[i].subMesh;

            mesh.min = glm::vec3 {FLT_MAX}, mesh.max = glm::vec3 {-FLT_MAX};
            for(size_t c = 0; c < m.m_vertexCount; c++){
                uint16_t index = indices[m.m_beginIndex + c];
                glm::vec3 vertexPos = position[index];

                mesh.min = glm::min(vertexPos, mesh.min);
                mesh.max = glm::max(vertexPos, mesh.max);
            }

            mesh.center = (mesh.min + mesh.max ) / 2.f;
            if (mesh.ffd == nullptr){
                mesh.ffd = std::make_shared<ffd>(mesh.min, mesh.max, mDevice, mRenderPass);
            }
            auto updateMinAABB = finalMatrix * glm::vec4 ( mesh.min, 1.f), updateMaxAABB = finalMatrix * glm::vec4 (mesh.max, 1.f);
            mesh.ffd->updateEveryFrame(mProj * mView, updateMinAABB, updateMaxAABB);

            if (Window::get()->intersect) {
                if (Window::get()->rayIntersectsAABB(updateMinAABB, updateMaxAABB, mSubMesh[i].distance)) {
                    ray = true;
                    if (mSubMesh[i].distance < minDistance){
                        minDistance = mSubMesh[i].distance;
                        nearestIndex = (int )i;
                    }
                }
            }

            ///fragment
            auto mmdFragUnifBuf = reinterpret_cast<MMDFragmentShaderUB*>(uniformBufPtr + sizeof (MMDFragmentShaderUB));
            auto mmdMaterial = mSubMesh[i].materialInfo;

            mmdFragUnifBuf->m_alpha = mmdMaterial.m_alpha;
            mmdFragUnifBuf->m_diffuse = mmdMaterial.m_diffuse;
            mmdFragUnifBuf->m_ambient = mmdMaterial.m_ambient;
            mmdFragUnifBuf->m_specular = mmdMaterial.m_specular;
            mmdFragUnifBuf->m_specularPower = mmdMaterial.m_specularPower;

            mmdFragUnifBuf->m_textureModes.x = mmdMaterial.m_textureModes.x;
            mmdFragUnifBuf->m_texAddFactor = mmdMaterial.m_texAddFactor;

            mmdFragUnifBuf->m_lightColor = mLightColor;
            glm::vec3 lightDir = lightManager::getDirectionLight()->getLightDirect();
            auto viewMaterial = glm::mat3 (mView);
            lightDir = viewMaterial * lightDir;
            mmdFragUnifBuf->m_lightDir = lightDir;

            mDevice.unmapMemory(uniformBuf->m_memory);
            uniformBuf->CopyBuffer(mSubMesh[i].mUniformBuf->getBuffer(), uniformSize);

            for(auto& updateSet : mUpdateDescriptorSet){
                updateSet();
            }
            mUpdateDescriptorSet.clear();


            if (size_t iCopy = i; !vkImguiManager::get()->getImguiTask(eToolbar).findTask(mSubMesh[i].imguiTransform)){
                mSubMesh[i].imguiTransform = vkImguiManager::get()->addRenderToToolbar([this, iCopy, mmdMaterial](){
                    auto& subMesh = mSubMesh[iCopy];
                    auto& currentPos = mSubMesh[iCopy].transform;
                    auto& scale = mSubMesh[iCopy].scale;
                    auto& mat = mSubMesh[iCopy].materialInfo;
                    auto& eff = mMaterials[iCopy]->getFragEffect();
                    auto& material = mMaterials[iCopy];
                    auto iN = [&](const std::string& name){
                        std::string r{name + "##" + std::to_string(iCopy)};
                        return r;
                    };

                    auto hasUniqueTex = material->getUniqueTex();
                    auto& hasEffControl = material->getEffControl();
                    if (hasUniqueTex) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));  // 红色
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.9f, 0.5f, 0.5f, 1.0f));  // 鼠标悬停时的颜色
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));  // 激活时的颜色
                    }


                    if (ImGui::CollapsingHeader(("Mesh " + std::to_string(iCopy)).c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Image((ImTextureID) mImguiTexDescriptor.getDescriptorSets()[iCopy], ImVec2(50, 50));
                        ImVec2 imgMin = ImGui::GetItemRectMin();
                        ImVec2 imgMax = ImGui::GetItemRectMax();
                        Window::get()->addToImgRects(iCopy, imgMin, imgMax);
                        ImGui::SameLine();

                        if (hasUniqueTex) {
                            ImGui::PopStyleColor(3);  // 弹出之前设置的三种颜色
                        }

                        if (Window::get()->getImgPath().first == iCopy) {
                            mUpdateDescriptorSet.emplace_back([&]() {
                                auto cmd = createTempCmdBuf();
                                material = std::make_shared<pmx_material>(Window::get()->getImgPath().second, cmd);
                                submitTempCmdBuf(cmd);

                                mMmdDescriptor.update(iCopy, {
                                        vk::DescriptorBufferInfo{
                                                mSubMesh[iCopy].mUniformBuf->getBuffer(),
                                                0,
                                                sizeof(MMDVertxShaderUB)
                                        },
                                        vk::DescriptorBufferInfo{
                                                mSubMesh[iCopy].mUniformBuf->getBuffer(),
                                                sizeof(MMDVertxShaderUB),
                                                sizeof(MMDFragmentShaderUB)
                                        },
                                        vk::DescriptorImageInfo{
                                                material->getTexture()->getSampler(),
                                                material->getTexture()->getImageView(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal
                                        },
                                        vk::DescriptorBufferInfo{
                                                material->getEffUnifBuf()->getBuffer(),
                                                0,
                                                sizeof(MMDFragEffect)
                                        },
                                        vk::DescriptorImageInfo{
                                                mMaterials[iCopy]->getTexture()->getSampler(),
                                                shadowMap::get()->getShadowMapView(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal
                                        },
                                        vk::DescriptorBufferInfo{
                                                mDirectionLightSpaceMatrixBuf->getBuffer(),
                                                0,
                                                sizeof(glm::mat4 )
                                        },
                                        vk::DescriptorBufferInfo{
                                                shadowMap::get()->getConfigurationBuf()->getBuffer(),
                                                0,
                                                sizeof (shadowMapConfiguration)
                                        }
                                });

                                mImguiTexDescriptor.update(iCopy, {
                                        vk::DescriptorImageInfo{
                                                material->getTexture()->getSampler(),
                                                material->getTexture()->getImageView(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal
                                        }});

                                material->setUniqueTex();

                                vkWarn("{0}", Window::get()->getImgPath().second);

                                Window::get()->clearImgPath();
                            });
                        }

                        if (ImGui::Button(iN("Change to the new texture").c_str())){
                            auto cmd = createTempCmdBuf();
                            mMaterials[iCopy] = std::make_shared<pmx_material>(mMmdModel->GetMaterials()[iCopy].m_texture, cmd);
                            submitTempCmdBuf(cmd);
                            material->setUniqueTex();
                        }

                        ImGui::SliderFloat3(iN("Translation").c_str(), &currentPos.x, -10.0f, 10.0f);
                        if (mUniformScale){
                            if (ImGui::SliderFloat3(iN("Scale").c_str(), &scale.x, -10.f, 10.f))
                                scale.y = scale.z = scale.x;
                        } else{
                            ImGui::SliderFloat3(iN("Scale").c_str(), &scale.x, -0.0f, 10.0f);
                        }
                        ImGui::SameLine();
                        ImGui::Checkbox(iN("Uniform").c_str(), &mUniformScale);

                        ImGui::SliderFloat(iN("Alpha").c_str(), &mat.m_alpha, 0.f, 1.f);
                        ImGui::SameLine();
                        ImGui::Checkbox(iN("##alpha").c_str(), &subMesh.alpha);
                        if (subMesh.alpha){
                            mat.m_alpha = 1.f;
                        } else { mat.m_alpha = 0.f; }

                        /// pmx -material
                        ImGui::SliderFloat(iN("Ao factor").c_str(), &eff.aoStrength, 0.f, 1.f);eff.darkPartStrength = mMMDFragEff.darkPartStrength;
                        eff.nums = mMMDFragEff.nums;
                        eff.base = mMMDFragEff.base;
                        eff.baseUpper = mMMDFragEff.baseUpper;
                        eff.range = mMMDFragEff.range;
                        eff.stepSize = mMMDFragEff.stepSize;
                        eff.stepOrSmoothStep = mMMDFragEff.stepOrSmoothStep;
//                        eff.maxSpecular = mMMDFragEff.maxSpecular;
//                        eff.specularColorStrength = mMMDFragEff.specularColorStrength;
//                        eff.specularPow = mMMDFragEff.specularPow;
//                        eff.roughnessCoefficient = mMMDFragEff.roughnessCoefficient;

                        if (ImGui::CollapsingHeader(iN("Independent effect control").c_str()), ImGuiTreeNodeFlags_DefaultOpen){
                            ImGui::Checkbox(iN("Enable Blur").c_str(), &hasEffControl.eBlur);
                            ImGui::SameLine();
                            ImGui::Checkbox(iN("Enable HSV").c_str(), &hasEffControl.eHSV);
                            ImGui::SameLine();
                            ImGui::Checkbox(iN("Enable RGB").c_str(), &hasEffControl.eColor);
                            ImGui::SameLine();
                            ImGui::Checkbox(iN("Enable Specular").c_str(), &hasEffControl.eSpecular);
                            if (hasEffControl.eBlur){
                                ImGui::SliderFloat(iN("Blur Radius").c_str(), &eff.blurRadius, 0.0f, 50.0f, "%.1f");
                                ImGui::SliderFloat(iN("Blur Strength").c_str(), &eff.blurStrength, 0.0f, 1.0f, "%.3f");
                            }
                            if (hasEffControl.eHSV){
                                ImGui::SliderFloat(iN("Saturation Factor").c_str(), &eff.saturationFactor, 0.0f, 10.f, "%.2f");
                            }
                            if (hasEffControl.eColor){
//                                if(ImGui::ColorPicker4(iN("RGB").c_str(), subMesh.color, ImGuiColorEditFlags_Float)){
//                                eff.r = subMesh.color[0];
//                                eff.g = subMesh.color[1];
//                                eff.b = subMesh.color[2];
//                                eff.a = subMesh.color[3];
//                                }
                                ImGui::SliderFloat(iN("R").c_str(), &eff.r, 0.0f, 1.f, "%.2f");
                                ImGui::SliderFloat(iN("G").c_str(), &eff.g, 0.0f, 1.f, "%.2f");
                                ImGui::SliderFloat(iN("B").c_str(), &eff.b, 0.0f, 1.f, "%.2f");
                                ImGui::SliderFloat(iN("A").c_str(), &eff.a, 0.0f, 1.f, "%.2f");
                            }
                            if (hasEffControl.eSpecular){
                                ImGui::SliderFloat(iN("Max Specular").c_str(), &eff.maxSpecular, 0.0f, 1.0f, "%.3f");
                                ImGui::SliderFloat(iN("Specular Color Strength").c_str(), &eff.specularColorStrength, 0.0f, 1.0f, "%.3f");
                                ImGui::SliderFloat(iN("Roughness Coefficient").c_str(), &eff.roughnessCoefficient, 0.0f, 0.01f, "%.3f");
                                ImGui::SliderFloat(iN("Specular Power").c_str(), &eff.specularPow, 0.0f, 10.0f, "%.3f");
                            }
                        }
                        if (!hasEffControl.eBlur){
                            eff.blurRadius = mMMDFragEff.blurRadius;
                            eff.blurStrength = mMMDFragEff.blurStrength;
                        }
                        if (!hasEffControl.eHSV){
                            eff.saturationFactor = mMMDFragEff.saturationFactor;
                            eff.hueAngle = mMMDFragEff.hueAngle;
                            eff.brightness = mMMDFragEff.brightness;
                        }
                        if (!hasEffControl.eColor){
//                            eff.r = subMesh.color[0];
//                            eff.g = subMesh.color[1];
//                            eff.b = subMesh.color[2];
//                            eff.a = subMesh.color[3];
                        }
                        if (!hasEffControl.eSpecular) {
                            eff.maxSpecular = mMMDFragEff.maxSpecular;
                            vkWarn(eff.maxSpecular);
                            eff.specularColorStrength = mMMDFragEff.specularColorStrength;
                            eff.specularPow = mMMDFragEff.specularPow;
                            eff.roughnessCoefficient = mMMDFragEff.roughnessCoefficient;
                        }
                    }


                });
            }

            mMaterials[i]->updateEffBuf();
        }

        if (mRayIntersect == nullptr)
            mRayIntersect = std::make_shared<ffd>(mDevice, mRenderPass);

        if (Window::get()->intersect){
            for(auto& mesh : mSubMesh){
                mesh.drawAABB = false;
                mMove_x = false;
                mMove_y = false;
                mMove_z = false;
            }
            check = true;
            mRayIntersect->updateRayEveryFrame({vkCamera::get()->getPos(), vkCamera::get()->getPos() + Window::get()->detectionMouseRay() * 10000.f});
            Window::get()->intersect = false;
        }


        if (ray && check){
            if (nearestIndex != -1){
                mSubMesh[nearestIndex].drawAABB = true;
                if (Window::get()->rayIntersectsXYZAABB(mSubMesh[nearestIndex].ffd->getX_AABBMinAndMax().first, mSubMesh[nearestIndex].ffd->getX_AABBMinAndMax().second)){
                    std::cout << "intersect x axis" << std::endl;
                    mMove_x = true;
                }
                if (Window::get()->rayIntersectsXYZAABB(mSubMesh[nearestIndex].ffd->getY_AABBMinAndMax().first, mSubMesh[nearestIndex].ffd->getY_AABBMinAndMax().second)){
                    std::cout << "intersect y axis" << std::endl;
                    mMove_y = true;
                }
                if (Window::get()->rayIntersectsXYZAABB(mSubMesh[nearestIndex].ffd->getZ_AABBMinAndMax().first, mSubMesh[nearestIndex].ffd->getZ_AABBMinAndMax().second)){
                    std::cout << "intersect z axis" << std::endl;
                    mMove_z = true;
                }
            }

            std::cout << "yes intersect AABB , the mesh index is  " << nearestIndex << std::endl;
            minDistance = std::numeric_limits<float>::max();
            nearestIndex = -1;
        } else if (check) {
            std::cout << "no intersect AABB" << std::endl;
        }


        Window::get()->updateXYZMoveDistance();
        for(auto& mesh : mSubMesh){
            if (mesh.drawAABB && Window::get()->mMoveXYZ && vkImguiRenderModel::get()->getCurrentModelTask() == eEditorModel){
                if (mMove_x){
                    mesh.transform += Window::MoveX() ;
                }
                if (mMove_y){
                    mesh.transform += Window::MoveY();
                }
                if (mMove_z){
                    mesh.transform += Window::MoveZ();
                }
            }
        }

        return *this;
    }

    pmx_context &pmx_context::draw(const vk::CommandBuffer& cmd) {
        vk::DeviceSize offsets[] = {0};
        cmd.bindVertexBuffers(0, 1, &mModelResource.mVertexBuffer->getBuffer(), offsets);
        cmd.bindIndexBuffer(mModelResource.mIndexBuffer->getBuffer(), 0, mModelResource.mIndexType);

        vk::Pipeline *currentMmdPipe = nullptr;
        for(uint32_t i = 0; auto& mesh : mSubMesh){
            const auto& mmdMaterial = mMmdModel->GetMaterials()[mesh.subMesh.m_materialID];
            if (mmdMaterial.m_alpha == 0.f)
                continue;

            std::vector<vk::DescriptorSet> mmdDescriptors{mMmdDescriptor.getDescriptorSets()[i], mMmdDescriptor.getDescriptorSets()[mMaterialCount]};
//            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, 1, &mMmdDescriptor.getDescriptorSets()[i], 0,
//                                   nullptr);
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, mmdDescriptors, {});

            vk::Pipeline* mmdPipeline = nullptr;
            if (mmdMaterial.m_bothFace)
                mmdPipeline = &mPipeline;

            if (currentMmdPipe != mmdPipeline){
                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *mmdPipeline);
                currentMmdPipe = mmdPipeline;
            }
            cmd.drawIndexed(mesh.subMesh.m_vertexCount, 1, mesh.subMesh.m_beginIndex, 0, 1);

            i++;
        }

        if(vkImguiRenderModel::get()->getCurrentModelTask() == eEditorModel){
            for (auto &mesh: mSubMesh) {
                if (mesh.drawAABB) {
                    mesh.ffd->drawHighLightFFD(cmd);
                    mesh.ffd->drawXYZ(cmd);
                } else {
                    if (vkImguiRenderModel::get()->getCurrentEditorModel() == eAllAABB){
                        mesh.ffd->drawFFD(cmd);
                    }
                }
            }

            if (mRayIntersect != nullptr) {
                mRayIntersect->drawRay(cmd);
            }
        }

        return *this;
    }

    pmx_context &pmx_context::drawDirectLightShadowMap(const vk::CommandBuffer &cmd) {
        vk::DeviceSize offsets[] = {0};
        cmd.bindVertexBuffers(0, 1, &mModelResource.mVertexBuffer->getBuffer(), offsets);
        cmd.bindIndexBuffer(mModelResource.mIndexBuffer->getBuffer(), 0, mModelResource.mIndexType);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowMap::get()->getShadowMapPipeline());
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowMap::get()->getShadowMapPipelineLayout(), 0,
                               mDirShadowMap.getSet().getDescriptorSets(), nullptr);

        for(auto& mesh : mSubMesh){
            const auto& mmdMaterial = mMmdModel->GetMaterials()[mesh.subMesh.m_materialID];
            if (mmdMaterial.m_alpha == 0.f)
                continue;

            cmd.drawIndexed(mesh.subMesh.m_vertexCount, 1, mesh.subMesh.m_beginIndex, 0, 1);
        }


        return *this;
    }


/////////

    pmx_context &pmx_context::loadPmxInputModels(const std::vector<Input> &inputModels) {
        for(const auto& input : inputModels){
            auto ext = saba::PathUtil::GetExt(input.m_modelPath);
            if (ext == "pmx"){
                auto pmxModel = std::make_unique<saba::PMXModel>();
                if (!pmxModel->Load(input.m_modelPath, mMmdDir)){
                    vkError("failed to load pmx file");
                }
                mMmdModel = std::move(pmxModel);
            }
            mMmdModel->InitializeAnimation();

            auto vmdAnim = std::make_unique<saba::VMDAnimation>();
            if (!vmdAnim->Create(mMmdModel)){
                vkError("failed to create VMD anim");
            }
            for(const auto& vmdPath : input.m_vmdPaths){
                saba::VMDFile vmdFile;
                if (!saba::ReadVMDFile(&vmdFile, vmdPath.c_str())){
                    vkError("failed to read vmd file");
                }
                if (!vmdAnim->Add(vmdFile)){
                    vkError("failed to add vmd anim");
                }
            }
            vmdAnim->SyncPhysics(0.f);
            mVmdAnimation = std::move(vmdAnim);
        }

        return *this;
    }

    vk::CommandBuffer pmx_context::createTempCmdBuf() {
        vk::CommandBufferAllocateInfo allocateInfo{mCommandPool, vk::CommandBufferLevel::ePrimary, 1};
        vk::CommandBuffer cmd = mDevice.allocateCommandBuffers(allocateInfo).front();

        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        cmd.begin(beginInfo);

        return cmd;
    }

    void pmx_context::submitTempCmdBuf(vk::CommandBuffer& cmd) {
        cmd.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmd);
        mvkInit->graphicsQueue().submit(submitInfo);
        mvkInit->graphicsQueue().waitIdle();
        mDevice.free(mCommandPool, cmd);
    }


//15195546346































} // vkPmx