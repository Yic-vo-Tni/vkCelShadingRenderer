//
// Created by lenovo on 3/22/2024.
//

#ifndef VKMMD_VK_OFFSCREEN_H
#define VKMMD_VK_OFFSCREEN_H

#include <ke_q/file_operation.h>

#include "niLou/vkPipeline.h"
#include "niLou/vkDescriptor.h"

#include "niLou/vkImage.h"

namespace yic {

    class vkOffScreen : public nonCopyable, public textureState{
    public:
        vkOffScreen& createOffScreenImage();
        vkOffScreen& createOffScreenImageView();
        vkOffScreen& createOffScreenFrameBuffers();
        vkOffScreen& createOffScreenRenderPass();
        vkOffScreen& createCmdPoolAndSemaphore();


    private:
        // init
        vk::Device mDevice;
        vk::PhysicalDevice mPhysicalDevice;
        vk::Extent2D mExtent;
        vk::Format mDepthFormat;

        // create
        vk::Image mDepthImage;
        vk::ImageView mDepthImageView;
        vk::DeviceMemory mDepthImageMemory;
        vk::DeviceMemory mOffMemory;
    };

} // yic

#endif //VKMMD_VK_OFFSCREEN_H
