//
// Created by lenovo on 4/12/2024.
//

#include "ffd.h"

#include <utility>

namespace yic {

    vk::Pipeline ffd::mFFDPipeline{};
    vk::Pipeline ffd::mHighLightPipeline{};
    vk::PipelineLayout ffd::mFFDPipelineLayout{};
    vkDescriptor ffd::mFFDDescriptor{};
    allocManager::bufUptr ffd::mMVPUniformBuf{};

    vk::Pipeline ffd::mXYZPipeline{};

    ffd::ffd(glm::vec3 min, glm::vec3 max, vk::Device device, vk::RenderPass renderPass) : mDevice(device), mFFDRenderPass(renderPass) {
//        std::cout << min.x << " " << min.y << " " << min.z << std::endl;
//        std::cout << max.x << " " << max.y << " " << max.z << std::endl;

        updateData(min, max);

        if (mFFDPipeline == nullptr){
            createFFDPipeline();
            createFFdDescriptor();
            createHighLightPipeline();
            createXYZPipeline();
        }

        createBuf();
    }

    ffd::ffd(vk::Device device, vk::RenderPass renderPass) {
        mDevice = device;
        mFFDRenderPass = renderPass;

        createRayPipeline();
        createRayDescriptor();
        createRayBuf();
    }

    void ffd::drawFFD(const vk::CommandBuffer& cmd) {
        std::vector<vk::DeviceSize> offsets = {0};
        std::vector<vk::Buffer> mBuffers{mVertBuf->getBuffer()};
        cmd.bindVertexBuffers(0, mBuffers, offsets);
        cmd.bindIndexBuffer(mIndexBuf->getBuffer(), 0, vk::IndexType::eUint32);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mFFDPipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mFFDPipelineLayout, 0, mFFDDescriptor.getDescriptorSets(),nullptr);

        cmd.drawIndexed(mIndices.size(), 1, 0, 0, 0);
    }

    void ffd::drawHighLightFFD(const vk::CommandBuffer &cmd) {
        std::vector<vk::DeviceSize> offsets = {0};
        std::vector<vk::Buffer> mBuffers{mVertBuf->getBuffer()};
        cmd.bindVertexBuffers(0, mBuffers, offsets);
        cmd.bindIndexBuffer(mIndexBuf->getBuffer(), 0, vk::IndexType::eUint32);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mHighLightPipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mFFDPipelineLayout, 0, mFFDDescriptor.getDescriptorSets(),nullptr);

        cmd.drawIndexed(mIndices.size(), 1, 0, 0, 0);
    }

    void ffd::drawXYZ(const vk::CommandBuffer &cmd) {
        std::vector<vk::DeviceSize > offsets = {0};
        std::vector<vk::Buffer> mBuffers{mXYZVertBuf->getBuffer()};
        cmd.bindVertexBuffers(0, mBuffers, offsets);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mXYZPipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mFFDPipelineLayout, 0, mFFDDescriptor.getDescriptorSets(),
                               nullptr);

        cmd.draw(6, 1, 0, 0);

        // x AABB
        mBuffers = std::vector<vk::Buffer>{mXAABBVertBuf->getBuffer()};
        cmd.bindVertexBuffers(0, mBuffers, offsets);
        cmd.bindIndexBuffer(mIndexBuf->getBuffer(), 0, vk::IndexType::eUint32);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mXYZPipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mFFDPipelineLayout, 0, mFFDDescriptor.getDescriptorSets(),nullptr);

        cmd.drawIndexed(mIndices.size(), 1, 0, 0, 0);

        // y AABB
        mBuffers = std::vector<vk::Buffer>{mYAABBVertBuf->getBuffer()};
        cmd.bindVertexBuffers(0, mBuffers, offsets);
        cmd.drawIndexed(mIndices.size(), 1, 0, 0, 0);

        // z AABB
        mBuffers = std::vector<vk::Buffer>{mZAABBVertBuf->getBuffer()};
        cmd.bindVertexBuffers(0, mBuffers, offsets);
        cmd.drawIndexed(mIndices.size(), 1, 0, 0, 0);
    }

    void ffd::drawRay(const vk::CommandBuffer &cmd) {
        std::vector<vk::DeviceSize> offsets = {0};
        std::vector<vk::Buffer> mBuffers{mVertRayBuf->getBuffer()};
        cmd.bindVertexBuffers(0, mBuffers, offsets);
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mRayPipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mRayPipelineLayout, 0, mRayDescriptor.getDescriptorSets(),
                               nullptr);
        cmd.draw(mRayVertices.size(), 1, 0, 0);
    }

    bool ffd::updateEveryFrame(glm::mat4 vp, glm::vec3 min, glm::vec3 max) {
        mMvp.vpMatrix = vp;

        updateData(min, max);
        mMVPUniformBuf->updateBuffer(mMvp);
        mVertBuf->updateBuffer(mVertices);
        mIndexBuf->updateBuffer(mIndices);

        mXYZVertBuf->updateBuffer(mXYZVertices);

        mXAABBVertBuf->updateBuffer(mXAABBVertices);
        mYAABBVertBuf->updateBuffer(mYAABBVertices);
        mZAABBVertBuf->updateBuffer(mZAABBVertices);

        return true;
    }

    bool ffd::updateRayEveryFrame(std::vector<glm::vec3> vertices) {
        mRayVertices = std::move(vertices);

        mVertRayBuf->updateBuffer(mRayVertices);

        return true;
    }

    bool ffd::createFFDPipeline() {
        mFFDDescriptor.addDescriptorSetLayout({
                                                      vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                              });

        mFFDPipelineLayout = mFFDDescriptor.getPipelineLayout();
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mFFDPipelineLayout, mFFDRenderPass};
        pipelineCombined.inputAssemblyState.setTopology(vk::PrimitiveTopology::eLineList)
                    .setPrimitiveRestartEnable(vk::False);
        pipelineCombined.rasterizationState
                .setFrontFace(vk::FrontFace::eCounterClockwise)
                .setCullMode(vk::CullModeFlagBits::eNone);
        pipelineCombined.addShader(ke_q::loadFile("v_ffd.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_ffd.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription({0, sizeof(glm::vec3 ), vk::VertexInputRate::eVertex});
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, 0},
                                                  });

        pipelineCombined.updateState();
        mFFDPipeline = pipelineCombined.createGraphicsPipeline();

        return true;
    }

    bool ffd::createRayPipeline() {
        mRayDescriptor.addDescriptorSetLayout({
                                                      vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                              });

        mRayPipelineLayout = mFFDDescriptor.getPipelineLayout();
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mRayPipelineLayout, mFFDRenderPass};
        pipelineCombined.inputAssemblyState.setTopology(vk::PrimitiveTopology::eLineList)
                .setPrimitiveRestartEnable(vk::False);
        pipelineCombined.rasterizationState
                .setLineWidth(3.f)
                .setCullMode(vk::CullModeFlagBits::eNone);
        pipelineCombined.addShader(ke_q::loadFile("v_ray.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_ray.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription({0, sizeof(glm::vec3 ), vk::VertexInputRate::eVertex});
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, 0},
                                                  });

        pipelineCombined.updateState();
        mRayPipeline = pipelineCombined.createGraphicsPipeline();

        return true;
    }

    bool ffd::createBuf() {
        mVertBuf = allocManager::build::bufUptr(sizeof (glm::vec3) * mVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
        mIndexBuf = allocManager::build::bufUptr(sizeof (uint32_t ) * mIndices.size(), vk::BufferUsageFlagBits::eIndexBuffer);

        mXYZVertBuf = allocManager::build::bufUptr(sizeof (XYZVert) * mXYZVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);

        mXAABBVertBuf = allocManager::build::bufUptr(sizeof (XYZVert) * mXAABBVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
        mYAABBVertBuf = allocManager::build::bufUptr(sizeof (XYZVert) * mYAABBVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
        mZAABBVertBuf = allocManager::build::bufUptr(sizeof (XYZVert) * mZAABBVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);

        return true;
    }

    bool ffd::createRayBuf() {
        mVertRayBuf = allocManager::build::bufUptr(sizeof(glm::vec3) * 100, vk::BufferUsageFlagBits::eVertexBuffer);

        return true;
    }

    bool ffd::createFFdDescriptor() {
        mMVPUniformBuf = allocManager::build::bufUptr(sizeof (glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer);

        mFFDDescriptor
                .createDescriptorPool()
                .createDescriptorSets()
                .addDescriptorSet({
                                          vk::DescriptorBufferInfo{mMVPUniformBuf->getBuffer(), 0, sizeof(glm::mat4)}
                                  })
                .update();

        return true;
    }

    bool ffd::createRayDescriptor() {
        if (mMVPUniformBuf == nullptr)
            mMVPUniformBuf = allocManager::build::bufUptr(sizeof (glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer);

        mRayDescriptor.createDescriptorPool()
                    .createDescriptorSets()
                    .addDescriptorSet({
                        vk::DescriptorBufferInfo{mMVPUniformBuf->getBuffer(), 0, sizeof(glm::mat4 )}
                    })
                    .update();

        return true;
    }

    bool ffd::updateData(glm::vec3 min, glm::vec3 max) {
        mVertices = {
                {min.x, min.y, min.z},
                {max.x, min.y, min.z},
                {max.x, max.y, min.z},
                {min.x, max.y, min.z},
                {min.x, min.y, max.z},
                {max.x, min.y, max.z},
                {max.x, max.y, max.z},
                {min.x, max.y, max.z}

        };
        mIndices = {
                0, 1, 1, 2, 2, 3, 3, 0,  // 底部
                4, 5, 5, 6, 6, 7, 7, 4,  // 顶部
                0, 4, 1, 5, 2, 6, 3, 7   // 侧面
        };

        glm::vec3 center = (min + max) * 0.5f;

        float length = glm::max(max.x - min.x, glm::max(max.y - min.y, max.z - min.z)) * (float)0.25;

        mXYZVertices = {
                {center, {1.f, 0.f, 0.f}},
                {center + glm::vec3 {length, 0.f, 0.f}, {1.f, 0.f, 0.f}},

                {center, {0.f, 1.f, 0.f}},
                {center + glm::vec3 {0.f, length, 0.f}, {0.f, 1.f, 0.f}},

                {center, {0.f, 0.f, 1.f}},
                {center + glm::vec3 {0.f, 0.f, length}, {0.f, 0.f, 1.f}},
        };

        mXAABBVertices = createXYZAABB(center, {center + glm::vec3{length, 0.f, 0.f}}, length * 0.2f, {1.f, 0.f, 0.f});
        mYAABBVertices = createXYZAABB(center, {center + glm::vec3 {0.f, length, 0.f}}, length * 0.2f, {0.f, 1.f, 0.f});
        mZAABBVertices = createXYZAABB(center, {center + glm::vec3{0.f, 0.f, length}}, length * 0.2f, {0.f, 0.f, 1.f});

        mX_AABBMinAndMax = {findMinPoint(mXAABBVertices), findMAXPoint(mXAABBVertices)};
        mY_AABBMinAndMax = {findMinPoint(mYAABBVertices), findMAXPoint(mYAABBVertices)};
        mZ_AABBMinAndMax = {findMinPoint(mZAABBVertices), findMAXPoint(mZAABBVertices)};


        return true;
    }

    bool ffd::createHighLightPipeline() {
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mFFDPipelineLayout, mFFDRenderPass};
        pipelineCombined.inputAssemblyState.setTopology(vk::PrimitiveTopology::eLineList)
                .setPrimitiveRestartEnable(vk::False);
        pipelineCombined.rasterizationState
                .setFrontFace(vk::FrontFace::eCounterClockwise)
                .setCullMode(vk::CullModeFlagBits::eNone);
        pipelineCombined.rasterizationState.setLineWidth(5.f);
        pipelineCombined.addShader(ke_q::loadFile("v_highlight.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_hightlight.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription({0, sizeof(glm::vec3 ), vk::VertexInputRate::eVertex});
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, 0},
                                                  });

        pipelineCombined.updateState();
        mHighLightPipeline = pipelineCombined.createGraphicsPipeline();

        return true;
    }

    bool ffd::createXYZPipeline() {
        graphicsPipelineGeneratorCombined pipelineCombined{mDevice, mFFDPipelineLayout, mFFDRenderPass};
        pipelineCombined.inputAssemblyState.setTopology(vk::PrimitiveTopology::eLineList)
                .setPrimitiveRestartEnable(vk::False);
        pipelineCombined.rasterizationState
                .setFrontFace(vk::FrontFace::eCounterClockwise)
                .setCullMode(vk::CullModeFlagBits::eNone);
        pipelineCombined.rasterizationState.setLineWidth(5.f);
        pipelineCombined.depthStencilState.setDepthTestEnable(vk::False)
                                        .setDepthWriteEnable(vk::False)
                                        .setDepthCompareOp(vk::CompareOp::eAlways)
                                        .setDepthBoundsTestEnable(vk::False)
                                        .setStencilTestEnable(vk::False);
        pipelineCombined.blendAttachmentStates.emplace_back(graphicsPipelineState::makePipelineColorBlendAttachments(vk::ColorComponentFlagBits::eR |vk::ColorComponentFlagBits::eG |
                                                                                                                               vk::ColorComponentFlagBits::eB |vk::ColorComponentFlagBits::eA,
                                                                                                                     false,
                                                                                                                     vk::BlendFactor::eOne, vk::BlendFactor::eZero,{},
                                                                                                                     vk::BlendFactor::eOne, vk::BlendFactor::eZero, {}));
        pipelineCombined.addShader(ke_q::loadFile("v_xyz.spv"), vk::ShaderStageFlagBits::eVertex);
        pipelineCombined.addShader(ke_q::loadFile("f_xyz.spv"), vk::ShaderStageFlagBits::eFragment);
        pipelineCombined.addBindingDescription(
                {0, sizeof(XYZVert ), vk::VertexInputRate::eVertex}
        );
        pipelineCombined.addAttributeDescriptions({
                                                          {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(XYZVert, pos)},
                                                          {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(XYZVert, color)}
                                                  });

        pipelineCombined.updateState();
        mXYZPipeline = pipelineCombined.createGraphicsPipeline();


        return true;
    }


    std::vector<ffd::XYZVert> ffd::createXYZAABB(const glm::vec3& p1, const glm::vec3& p2, float thickness, const glm::vec3& color) {
        glm::vec3 center = (p1 + p2) * 0.5f;
        glm::vec3 direction = glm::normalize(p2 - p1);
        float length = glm::distance(p1, p2);

        glm::vec3 up = glm::vec3(0, 1, 0);

        if (std::abs(direction.y) > std::abs(direction.x)){
            up = glm::vec3 (1.f, 0.f, 0.f);
        } else{
            up = glm::vec3 (0.f, 1.f, 0.f);
        }

        glm::vec3 right = glm::cross(direction, up);
        up = glm::cross(right, direction);

        right = glm::normalize(right);
        up = glm::normalize(up);

        float halfWidth = thickness * 0.5f;
        float halfHeight = thickness * 0.5f;

        std::vector<XYZVert> box = {
                {center - halfWidth * right - halfHeight * up + 0.5f * length * direction, color},
                {center + halfWidth * right - halfHeight * up + 0.5f * length * direction, color},
                {center + halfWidth * right + halfHeight * up + 0.5f * length * direction, color},
                {center - halfWidth * right + halfHeight * up + 0.5f * length * direction, color},
                {center - halfWidth * right - halfHeight * up - 0.5f * length * direction, color},
                {center + halfWidth * right - halfHeight * up - 0.5f * length * direction, color},
                {center + halfWidth * right + halfHeight * up - 0.5f * length * direction, color},
                {center - halfWidth * right + halfHeight * up - 0.5f * length * direction, color}
        };

        return box;
    }

    glm::vec3 ffd::findMinPoint(const std::vector<ffd::XYZVert> &vertices) {
        glm::vec3 minPoint = vertices[0].pos;
        for (const auto& vert : vertices) {
            minPoint.x = std::min(minPoint.x, vert.pos.x);
            minPoint.y = std::min(minPoint.y, vert.pos.y);
            minPoint.z = std::min(minPoint.z, vert.pos.z);
        }
        return minPoint;
    }

    glm::vec3 ffd::findMAXPoint(const std::vector<ffd::XYZVert> &vertices) {
        glm::vec3 maxPoint = vertices[0].pos;
        for (const auto& vert : vertices) {
            maxPoint.x = std::max(maxPoint.x, vert.pos.x);
            maxPoint.y = std::max(maxPoint.y, vert.pos.y);
            maxPoint.z = std::max(maxPoint.z, vert.pos.z);
        }
        return maxPoint;
    }


} // yic