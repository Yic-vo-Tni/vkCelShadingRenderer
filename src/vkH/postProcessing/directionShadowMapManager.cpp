//
// Created by lenovo on 5/5/2024.
//

#include "directionShadowMapManager.h"

namespace yic {

    directionShadowMapManager::directionShadowMapManager() {
        mLightMatrixBuf = allocManager::build::bufSptr(sizeof (glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer);

        mShadowMapDescriptor.addDescriptorSetLayout({
                                                            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                                                    });
        // shadow map set
        mShadowMapDescriptor.increaseMaxSets( ).createDescriptorPool();

        mShadowMapDescriptor.pushBackDesSets();
        mShadowMapDescriptor.updateDescriptorSet({
                                                         vk::DescriptorBufferInfo{
                                                                 mLightMatrixBuf->getBuffer(),
                                                                 0,
                                                                 sizeof(glm::mat4 )
                                                         },
                                                 });
    }

    void directionShadowMapManager::update(const glm::mat4 &modelMatrix) {
        mvpMatrix = lightManager::getDirectionLight()->getLightSpaceMatrix() * modelMatrix;

        mLightMatrixBuf->updateBuffer(mvpMatrix);
    }


} // yic