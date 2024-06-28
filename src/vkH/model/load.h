//
// Created by lenovo on 4/22/2024.
//

#ifndef VKMMD_LOAD_H
#define VKMMD_LOAD_H

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "niLou/vkImage.h"
#include "ke_q/vk_allocator.h"
#include "ke_q/file_operation.h"

#include "niLou/vkDescriptor.h"

#include "postProcessing/shadowMap.h"
#include "illuminant/lightBase.h"

#include "modelData.h"

namespace yic {


    class load {
    public:
        static void init(vk::Device device, vk::Queue queue);
        explicit load(const std::string& path);

        [[nodiscard]] inline auto& getYicModel() const { return mModel;}
        [[nodiscard]] inline auto& getPath() const { return mPath;}
        [[nodiscard]] inline auto& getDescriptors() const { return mDescriptor.getDescriptorSets();}
        [[nodiscard]] inline auto& getLightMatrixBuf() { return mLightSpaceMatrixBuf;}

        void renderObjControls();
        bool updateDescriptor(const yicModel& model);
    private:
        yicModel loadModel(const std::string& path);
        void processNode(aiNode* node, const aiScene* scene, yicModel& model);
        void processMesh(aiMesh* mesh, const aiScene* scene, yicObject& object);
        void secondaryProcessMesh(yicModel& model);

        vk::CommandBuffer createTempCmdBuf();
        void submitTempCmdBuf(vk::CommandBuffer& cmd);
    private:
        static vk::Device mDevice;
        static vk::Queue mGraphicsQueue;
        static vk::CommandPool mCmdPool;

        std::string mPath{};
        std::string mExtension{};
        std::filesystem::path mModelDirectory{};
        vkDescriptor mDescriptor{};
        yicModel mModel{};

        allocManager::bufSptr mLightSpaceMatrixBuf;

    public:
        bool mDelete{false};

    };

} // yic

#endif //VKMMD_LOAD_H
