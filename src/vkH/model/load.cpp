//
// Created by lenovo on 4/22/2024.
//

#include "load.h"
#include "log/Log.h"

namespace yic {

    vk::Device load::mDevice{};
    vk::Queue load::mGraphicsQueue{};
    vk::CommandPool load::mCmdPool{};

    load::load(const std::string &path) : mPath(path) {
        mModelDirectory = std::filesystem::path{path}.parent_path();
        auto lastDotPos = path.find_last_of('.');
        if (lastDotPos != std::string ::npos){
            mExtension = path.substr(lastDotPos + 1);
        } else{
            mExtension = "";
        }
        mModel = loadModel(path);
        updateDescriptor(mModel);
    }

    void load::init(vk::Device device, vk::Queue queue) {
        mDevice = device;
        mGraphicsQueue = queue;

        mCmdPool = mDevice.createCommandPool(vk::CommandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer});
    }


    yicModel load::loadModel(const std::string &path) {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                                   aiProcess_FlipUVs | aiProcess_FlipWindingOrder);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            vkError("assimp load model error : " + std::string(importer.GetErrorString()));
        }

        yicModel model;
        processNode(scene->mRootNode, scene, model);
        secondaryProcessMesh(model);
        return model;
    }

    void load::processNode(aiNode *node, const aiScene *scene,  yicModel& model) {
        if (node->mNumMeshes > 0) {
            yicObject obj;
            obj.mvpBuf = allocManager::build::bufSptr(sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer);

            for (uint32_t i = 0; i < node->mNumMeshes; i++) {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                processMesh(mesh, scene, obj);
            }

            model.modelAABB.min = glm::min(model.modelAABB.min, obj.objAABB.min);
            model.modelAABB.max = glm::max(model.modelAABB.max, obj.objAABB.max);

            vkWarn("{0}, {1}, {2}", model.modelAABB.min.x, model.modelAABB.min.y, model.modelAABB.min.z);

            model.objects.emplace_back(obj);
        }

        for(uint32_t i = 0; i < node->mNumChildren; i++){
            processNode(node->mChildren[i], scene, model);
        }
    }

    void load::processMesh(aiMesh *mesh, const aiScene *scene, yicObject& object) {
        yicSubMesh subMesh;
        auto& aabb = subMesh.meshAABB;

        for(uint32_t i = 0; i < mesh->mNumVertices; i++){
            yicVertex vertex{};

            auto rotationMat = glm::mat4 (1.f);
            if (mExtension == "fbx"){
                rotationMat = glm::rotate(glm::mat4 (1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
            }

            auto p = mesh->mVertices[i];
            glm::vec4 pos = glm::vec4 {p.x, p.y, p.z, 1.f} * rotationMat;
            vertex.pos = {pos.x, pos.y, pos.z};

            aabb.min = glm::min(vertex.pos, aabb.min);
            aabb.max = glm::max(vertex.pos, aabb.max);

            auto n = mesh->mNormals[i];
            if (mesh->HasNormals()){
                glm::vec4 nor = glm::vec4 {n.x, n.y, n.z, 0.f} * rotationMat;
                vertex.nor = {nor.x, nor.y, nor.z};
            }

            auto uv = mesh->mTextureCoords[0][i];
            if (mesh->mTextureCoords[0]){
                vertex.uv = {uv.x, uv.y};
            }

            subMesh.vertices.emplace_back(vertex);
        }

        object.objAABB.min = glm::min(object.objAABB.min, aabb.min);
        object.objAABB.max = glm::max(object.objAABB.max, aabb.max);
        //vkWarn("{0}, {1}, {2}", object.objAABB.min.x, object.objAABB.min.y, object.objAABB.min.z);

        for(uint32_t i = 0; i < mesh->mNumFaces; i++){
            auto face = mesh->mFaces[i];
            for(uint32_t j = 0; j < face.mNumIndices; j++){
                subMesh.indices.emplace_back(face.mIndices[j]);
            }
        }

        if (mesh->mMaterialIndex >= 0){
            aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
            aiString path;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS){
                std::filesystem::path texturePath(path.C_Str());

                std::filesystem::path texFilename = texturePath.filename();

                std::filesystem::path newPath = mModelDirectory / texFilename;
                vkWarn(newPath.string());
                subMesh.diffuseTex = std::make_shared<vkImage>(newPath.string());

                auto cmd = createTempCmdBuf();
                subMesh.diffuseTex->configureImageForRender(cmd);
                submitTempCmdBuf(cmd);

            } else{
                subMesh.diffuseTex = std::make_shared<vkImage>(R"(F:\Yicvot\Yicvot\src\pictures\2.png)");

                auto cmd = createTempCmdBuf();
                subMesh.diffuseTex->configureImageForRender(cmd);
                submitTempCmdBuf(cmd);
            }
        } else{
            subMesh.diffuseTex = std::make_shared<vkImage>(R"(F:\Yicvot\Yicvot\src\pictures\2.png)");

            auto cmd = createTempCmdBuf();
            subMesh.diffuseTex->configureImageForRender(cmd);
            submitTempCmdBuf(cmd);
        }

        subMesh.vertBuf = allocManager::build::bufSptr(sizeof (yicVertex) * subMesh.vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
        subMesh.indexBuf = allocManager::build::bufSptr(sizeof (uint32_t ) * subMesh.indices.size(), vk::BufferUsageFlagBits::eIndexBuffer);

        subMesh.indexBuf->updateBuffer(subMesh.indices);

        object.subMeshes.emplace_back(subMesh);
    }

    void load::secondaryProcessMesh(yic::yicModel &model) {
        auto& max = model.modelAABB.max;
        auto& min = model.modelAABB.min;
        float width = max.x - min.x,
            height = max.y - min.y,
            depth = max.z - min.z;

        auto maxDimension = std::max({width, height, depth});
        vkWarn("{0}, {1}, {2}", width, height, depth);
        auto targetMaxSize = 30.f;

        auto scale = targetMaxSize / maxDimension;

        for(auto& obj : model.objects){
            for(auto& mesh : obj.subMeshes){
                for(auto& vert : mesh.vertices){
                    auto& p = vert.pos;
                    if (maxDimension > targetMaxSize){
                        vert.pos = {
                                (p.x - min.x - width / 2) * scale,
                                (p.y - min.y - height / 2) * scale,
                                (p.z - min.z - depth / 2) * scale
                        };
                    }
                }

                mesh.vertBuf->updateBuffer(mesh.vertices);
            }
        }

    }

    void load::renderObjControls() {
        ImGui::SliderFloat3(("Translation##" + mPath).c_str(), &mModel.translation.x, -30.0f, 30.0f);
        mModel.transform = glm::translate(glm::mat4 (1.f), mModel.translation);
        ImGui::SliderFloat(("Scale##" + mPath).c_str(), &mModel.uniformScale, 0.1f, 10.f);
        mModel.transform = glm::scale(mModel.transform, glm::vec3 {mModel.uniformScale});

        ImGui::Spacing();
        if (ImGui::Button(("Delete##" + mPath).c_str())){
            mDelete = true;
        }
    }

    bool load::updateDescriptor(const yicModel& model) {
        auto& objects = model.objects;

        mDescriptor.addDescriptorSetLayout({
                                                   vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                                   vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                                   vk::DescriptorSetLayoutBinding{2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                                   vk::DescriptorSetLayoutBinding{3, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                                   vk::DescriptorSetLayoutBinding{4, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
                                           });

        mLightSpaceMatrixBuf = allocManager::build::bufSptr(sizeof (glm::mat4 ), vk::BufferUsageFlagBits::eUniformBuffer);

        uint32_t size = 0;
        for(const auto& obj : objects){
            size += obj.subMeshes.size();
        }

        mDescriptor.increaseMaxSets(size - 1 ).createDescriptorPool();

        for (const auto &obj: objects) {
            for (const auto &subMesh: obj.subMeshes) {
                mDescriptor.pushBackDesSets();
                mDescriptor.updateDescriptorSet({
                                                        vk::DescriptorBufferInfo{
                                                                obj.mvpBuf->getBuffer(),
                                                                0,
                                                                sizeof(glm::mat4)
                                                        },
                                                        vk::DescriptorImageInfo{
                                                                subMesh.diffuseTex->getSampler(),
                                                                subMesh.diffuseTex->getImageView(),
                                                                vk::ImageLayout::eShaderReadOnlyOptimal
                                                        },
                                                        vk::DescriptorImageInfo{
                                                                subMesh.diffuseTex->getSampler(),
                                                                shadowMap::get()->getShadowMapView(),
                                                                vk::ImageLayout::eShaderReadOnlyOptimal
                                                        },
                                                        vk::DescriptorBufferInfo{
                                                            mLightSpaceMatrixBuf->getBuffer(),
                                                            0,
                                                            sizeof(glm::mat4 )
                                                        },
                                                        vk::DescriptorBufferInfo{
                                                            shadowMap::get()->getConfigurationBuf()->getBuffer(),
                                                            0,
                                                            sizeof (shadowMapConfiguration)
                                                        }

                                                });
            }
        }

        return true;
    }

    vk::CommandBuffer load::createTempCmdBuf() {
        vk::CommandBufferAllocateInfo allocateInfo{mCmdPool, vk::CommandBufferLevel::ePrimary, 1};
        vk::CommandBuffer cmd = mDevice.allocateCommandBuffers(allocateInfo).front();

        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        cmd.begin(beginInfo);

        return cmd;
    }

    void load::submitTempCmdBuf(vk::CommandBuffer& cmd) {
        cmd.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmd);
        mGraphicsQueue.submit(submitInfo);
        mGraphicsQueue.waitIdle();
        mDevice.free(mCmdPool, cmd);
    }


} // yic