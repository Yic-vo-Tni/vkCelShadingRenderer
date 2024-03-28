//
// Created by lenovo on 1/4/2024.
//

#ifndef VULKAN_VKIMGUI_H
#define VULKAN_VKIMGUI_H

#include "modelTransformManager.h"
#include "nfd.h"

namespace yic {

    struct imguiContext{
        GLFWwindow* window{};
        vk::Instance instance;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;
        vk::Queue queue;
        vk::RenderPass renderPass;
        vk::DescriptorPool descriptorPool;
        uint32_t imageCount{};
    };

    class vkImgui : public nonCopyable{
    public:
        explicit vkImgui(imguiContext& imguiContext);
        ~vkImgui();

        void beginRenderImgui();
        void endRenderImgui(const vk::CommandBuffer& cmd);

        void fixedFrame(vk::Extent2D extent);
        void vkRenderWindow();
        void RenderViewWindow(vk::DescriptorSet set = {}, ImVec2 imageSize = {});

       vkImgui& setShowDemo(bool demo = true){ mShowDemo = demo; return *this;};
    private:
        bool mShowDemo{true};
        vk::Extent2D mExtent;
        imguiContext& mImguiContext;

        glm::vec3 translation{1.f, 0.f, 0.f};
    };



} // yic

#endif //VULKAN_VKIMGUI_H
