//
// Created by lenovo on 4/12/2024.
//

#ifndef VKMMD_FFD_H
#define VKMMD_FFD_H

#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"
#include "ke_q/vk_allocator.h"
#include "ke_q/file_operation.h"

namespace yic {

    class ffd {
        struct MVP{
            glm::mat4 vpMatrix;
        };
        struct XYZVert{
            glm::vec3 pos;
            glm::vec3 color;
        };
    public:
        ffd(glm::vec3 min, glm::vec3 max, vk::Device device, vk::RenderPass renderPass);
        ffd(vk::Device device, vk::RenderPass renderPass);

        bool updateEveryFrame(glm::mat4 vp, glm::vec3 min, glm::vec3 max);
        void drawFFD(const vk::CommandBuffer& cmd);
        void drawHighLightFFD(const vk::CommandBuffer& cmd);
        void drawXYZ(const vk::CommandBuffer& cmd);

        bool updateRayEveryFrame(std::vector<glm::vec3> vertices);
        void drawRay(const vk::CommandBuffer& cmd);

        [[nodiscard]] inline auto& getX_AABBMinAndMax() const { return mX_AABBMinAndMax;}
        [[nodiscard]] inline auto& getY_AABBMinAndMax() const { return mY_AABBMinAndMax;}
        [[nodiscard]] inline auto& getZ_AABBMinAndMax() const { return mZ_AABBMinAndMax;}
    private:
        bool createFFDPipeline();
        bool createHighLightPipeline();
        bool createXYZPipeline();
        bool createBuf();
        bool createFFdDescriptor();
        std::vector<XYZVert> createXYZAABB(const glm::vec3& p1, const glm::vec3& p2, float thickness, const glm::vec3& color);
        bool updateData(glm::vec3 min, glm::vec3 max);

        bool createRayPipeline();
        bool createRayBuf();
        bool createRayDescriptor();

        glm::vec3 findMinPoint(const std::vector<ffd::XYZVert>& vertices);
        glm::vec3 findMAXPoint(const std::vector<ffd::XYZVert>& vertices);


    private:
        static vk::Pipeline mFFDPipeline;
        static vk::Pipeline mHighLightPipeline;
        static vk::PipelineLayout mFFDPipelineLayout;
        static vkDescriptor mFFDDescriptor;
        static allocManager::bufUptr mMVPUniformBuf;

        static vk::Pipeline mXYZPipeline;

        vk::Device mDevice{};
        vk::RenderPass mFFDRenderPass{};

        //
        vk::Pipeline mRayPipeline{};
        vk::PipelineLayout mRayPipelineLayout{};
        vkDescriptor mRayDescriptor{};


        std::vector<glm::vec3> mVertices{};
        std::vector<uint32_t > mIndices{};

        std::vector<glm::vec3> mRayVertices{};

        std::vector<XYZVert> mXYZVertices{};
        std::vector<XYZVert> mXAABBVertices{}, mYAABBVertices{}, mZAABBVertices{};

        std::pair<glm::vec3, glm::vec3> mX_AABBMinAndMax{};
        std::pair<glm::vec3, glm::vec3> mY_AABBMinAndMax{};
        std::pair<glm::vec3, glm::vec3> mZ_AABBMinAndMax{};

        MVP mMvp{};

        allocManager::bufUptr mVertBuf{}, mIndexBuf{};

        allocManager::bufUptr mXYZVertBuf{};
        allocManager::bufUptr mXAABBVertBuf{}, mYAABBVertBuf{}, mZAABBVertBuf{};

        allocManager::bufUptr mVertRayBuf{};
    };

} // yic

#endif //VKMMD_FFD_H
