//
// Created by lenovo on 3/22/2024.
//

#ifndef VKMMD_VK_OFFSCREEN_H
#define VKMMD_VK_OFFSCREEN_H

#include <ke_q/file_operation.h>

#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"

namespace yic {

    class vk_offscreen : public nonCopyable{
    public:
        vk_offscreen& createOffscreenPipeline();
        vk_offscreen& updateDescriptor();

    private:
        vk::Device mDevice;
        vk::RenderPass mOffScreenRenderPass;
        vk::Sampler mSampler;

        vk::Pipeline mOffScreenGraphicsPipeline;
        vkDescriptor mOffScreenDescriptor;
    };

} // yic

#endif //VKMMD_VK_OFFSCREEN_H
