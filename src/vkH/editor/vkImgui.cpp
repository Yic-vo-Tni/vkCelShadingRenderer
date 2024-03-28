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
        style.WindowPadding = ImVec2(0.f, 0.f);
        style.WindowBorderSize = 0.f;

        ImGui::Begin("Yicvot", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar);
        {
            if (ImGui::BeginMenuBar()){
                if (ImGui::BeginMenu("File")){

                    if (ImGui::MenuItem("load pmx model", "Ctrl+P")) {
                        nfdchar_t *outPath{};
                        nfdresult_t nfdresult = NFD_OpenDialog("pmx;fbx", NULL, &outPath);

                        if (nfdresult == NFD_OKAY) {
                            modelTransformManager::get()->addPmxModel(outPath);
                            modelTransformManager::get()->setLoadPmxModel();
                            printf("select model successfully: %s\n", outPath);

                            free(outPath);
                        } else if (nfdresult == NFD_CANCEL) {
                            puts("cancel");
                        } else {
                            printf("error: %s\n", NFD_GetError());
                        }
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

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
        yStyle.WindowRounding = 5.0f; // 设置窗口角的圆滑度
        yStyle.FrameRounding = 3.0f; // 设置按钮和滑动条的圆滑度
        yStyle.ItemSpacing = ImVec2(10, 10); // 设置元素之间的间距
        ImGui::Begin("Toolbar");
        {
            if (modelTransformManager::get()->isRenderPmxModel()){
                auto currentPos = modelTransformManager::get()->getPos();
                ImGui::SliderFloat3("Translation", &currentPos.x, -10.0f, 10.0f); // 允许用户控制位移
                modelTransformManager::get()->setPosition(currentPos);

                if (ImGui::Button("Load vmd Anim", ImVec2(100, 20))){
                    nfdchar_t *outPath{};
                    nfdresult_t nfdresult = NFD_OpenDialog("vmd;fbx", NULL, &outPath);

                    if (nfdresult == NFD_OKAY){
                        modelTransformManager::get()->setLoadPmxModel();
                        //modelTransformManager::get()->setRenderPmxModel();
                        modelTransformManager::get()->addPmxVMDAnim(outPath);
                        printf("select vmd successfully: %s\n", outPath);

                        free(outPath);
                    } else if (nfdresult == NFD_CANCEL){
                        puts("cancel");
                    } else {
                        printf("error: %s\n", NFD_GetError());
                    }
                }
            }

            if (modelTransformManager::get()->isLoadVMDAnim()) {
                ImGui::SameLine();

                if (ImGui::Button(modelTransformManager::get()->playAnimVMD() ? "Pause" : "Play", ImVec2(100, 20))) {
                    modelTransformManager::get()->setAnimPlay();
                }
                ImGui::SameLine();
                // ImGui::Dummy(ImVec2(10, 0));
                ImGui::Text("Animation Control");

                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space))){
                    modelTransformManager::get()->setAnimPlay();
                }
            }
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