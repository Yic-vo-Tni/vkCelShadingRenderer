//
// Created by lenovo on 1/4/2024.
//

#include "vkImgui.h"

namespace yic {

    vkImgui::vkImgui(yic::imguiContext &imguiContext) : mImguiContext(imguiContext){
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::GetIO().IniFilename = imGuiIniPath "imgui.ini";

        ImGui_ImplGlfw_InitForVulkan(mImguiContext.window, false);

        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = mImguiContext.instance;
        info.PhysicalDevice = mImguiContext.physicalDevice;
        info.Device = mImguiContext.device;
        info.Queue = mImguiContext.queue;
        info.ImageCount = mImguiContext.imageCount;
        info.MinImageCount = mImguiContext.imageCount;
        info.DescriptorPool = mImguiContext.descriptorPool;
        ImGui_ImplVulkan_Init(&info, mImguiContext.renderPass);

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    vkImgui::~vkImgui() {
        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void vkImgui::beginRenderImgui() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (mShowDemo) ImGui::ShowDemoWindow(&mShowDemo);
    }

    void vkImgui::endRenderImgui(const vk::CommandBuffer& cmd) {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

    void vkImgui::fixedFrame(vk::Extent2D extent){
        if (mExtent != extent){
            mExtent = extent;
            ImVec2 imguiWindowSize((float) extent.width, (float) extent.height);
            ImVec2 imguiWindowPos(0, 0);
            ImGui::SetNextWindowSize(imguiWindowSize);
            ImGui::SetNextWindowPos(imguiWindowPos);
        }

        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg].w = 0.f;
        ImGui::Begin("Yicvot", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse) | ImGuiWindowFlags_NoDecoration |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id);
        }
        ImGui::End();

    }

    void vkImgui::vkRenderWindow() {
        ImGuiStyle &xStyle = ImGui::GetStyle();
        xStyle.Colors[ImGuiCol_WindowBg].w = 0.0f;
        ImGui::Begin("Main Render", nullptr, ImGuiWindowFlags_NoBackground);
        {

        }
        ImGui::End();

        ImGuiStyle &yStyle = ImGui::GetStyle();
        yStyle.Colors[ImGuiCol_WindowBg].w = 1.f;
        ImGui::Begin("Toolbar");
        {

        }
        ImGui::End();
    }

    void vkImgui::RenderViewWindow(vk::DescriptorSet set, ImVec2 imageSize) {
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg].w = 0.5f;
        ImGui::Begin("View");
        {
            if (set != nullptr)
                ImGui::Image((ImTextureID) set, imageSize);
        }
        ImGui::End();
    }

} // yic












//        //ImGui::DockSpace(ImGui::GetID("yicvot"), ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None);
//        //        style.Colors[ImGuiCol_DockingEmptyBg].w = 0.2f;


//            ImGuiID dock_main_id = ImGui::GetID("MyDockSpace");
//            ImGui::DockBuilderRemoveNode(dock_main_id);  // 清除现有的dock布局
//            ImGui::DockBuilderAddNode(dock_main_id, ImGuiDockNodeFlags_DockSpace);  // 添加一个新的dock节点
//            ImGui::DockBuilderSetNodeSize(dock_main_id, ImGui::GetMainViewport()->Size);
//
//            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.5f, nullptr, &dock_main_id);
//            ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.5f, nullptr, &dock_main_id);
//
//            //ImGui::DockBuilderDockWindow("Right Window", dock_id_left);
//            ImGui::DockBuilderDockWindow("x", dock_id_right);
//
//            ImGui::DockBuilderFinish(dock_main_id);