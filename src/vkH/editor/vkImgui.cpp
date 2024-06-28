//
// Created by lenovo on 1/4/2024.
//

#ifdef _MSC_VER
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#endif

#include "vkImgui.h"

namespace yic {

    vkImgui::vkImgui(yic::imguiContext &imguiContext) : mImguiContext(imguiContext){
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
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
        //style.WindowRounding = 50.f;
        //style.ChildRounding = 50.f;
        style.FramePadding = ImVec2(15.f, 15.f);
        //style.FrameRounding = 5.f;
        style.ItemSpacing = ImVec2(20.f, 20.f);
//        style.ItemInnerSpacing = ImVec2(10.f, 10.f);
    //    style.PopupBorderSize = 1.f;
        //style.PopupRounding = 50.f;
        style.TabRounding = 4;
        style.ScrollbarRounding = 9;
        style.WindowRounding = 7;
        style.GrabRounding = 3;
        style.FrameRounding = 3;
        style.PopupRounding = 4;

        style.FrameBorderSize = 0.f;

      //  style.ChildRounding = 4;

//        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.1f);
//        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
        auto &colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
        //colors[ImGuiCol_MenuBarBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_MenuBarBg] = ImVec4{0.06f, 0.06f, 0.11f, 1.0f};

// Border
        colors[ImGuiCol_Border] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
        colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.24f};

// Text
        colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
        colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

// Headers
        //colors[ImGuiCol_Header] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
        colors[ImGuiCol_Header] = ImVec4{0.33f, 0.33f, 0.47, 1.0f};
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
        colors[ImGuiCol_HeaderActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Buttons
        colors[ImGuiCol_Button] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
        colors[ImGuiCol_ButtonHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
        colors[ImGuiCol_ButtonActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_CheckMark] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};

// Popups
        colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.13f, 0.92f};

// Slider
        colors[ImGuiCol_SliderGrab] = ImVec4{0.44f, 0.37f, 0.61f, 0.54f};
        colors[ImGuiCol_SliderGrabActive] = ImVec4{0.74f, 0.58f, 0.98f, 0.54f};

// Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{0.13f, 0.13, 0.17, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
        colors[ImGuiCol_FrameBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Tabs
        colors[ImGuiCol_Tab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_TabHovered] = ImVec4{0.24, 0.24f, 0.32f, 1.0f};
        colors[ImGuiCol_TabActive] = ImVec4{0.2f, 0.22f, 0.27f, 1.0f};
        colors[ImGuiCol_TabUnfocused] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Title
        colors[ImGuiCol_TitleBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_TitleBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
        colors[ImGuiCol_ScrollbarGrab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};

// Seperator
        colors[ImGuiCol_Separator] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
        colors[ImGuiCol_SeparatorHovered] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
        colors[ImGuiCol_SeparatorActive] = ImVec4{0.84f, 0.58f, 1.0f, 1.0f};

// Resize Grip
        colors[ImGuiCol_ResizeGrip] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
        colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.74f, 0.58f, 0.98f, 0.29f};
        colors[ImGuiCol_ResizeGripActive] = ImVec4{0.84f, 0.58f, 1.0f, 0.29f};

// Docking
        colors[ImGuiCol_DockingPreview] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};


        ImGui::Begin("Yicvot", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar);
//        ImGui::Begin("Yicvot", nullptr);
        {
            if (ImGui::BeginMenuBar()){
                if (ImGui::BeginMenu("File")) {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});
                    if (ImGui::MenuItem("##e")){}
                    ImGui::PopStyleVar();

                    if (ImGui::MenuItem("open project")) {
                        auto j = js::vkProject::loadToUserSpecifiedLocation();
                        if (j != nullptr) {
                            modelTransformManager::get()->setInputModels(js::vkProject::pmxFromJson(j));
                            modelTransformManager::get()->setLoadPmxModel();
                        }
                    }
                    if (ImGui::MenuItem("save project")) {
                        auto j = js::vkProject::pmxToJson(modelTransformManager::get()->getInputModels().front());
                        js::vkProject::saveToUserSpecifiedLocation(j);
                    }

                    if (ImGui::MenuItem("load pmx character", "Ctrl+P")) {
#ifdef _MSC_VER
                        OPENFILENAME ofn;       // common dialog box structure
                        CHAR szFile[260] = {0};  // buffer for file name
                        // Initialize OPENFILENAME
                        ZeroMemory(&ofn, sizeof(ofn));
                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = nullptr;
                        ofn.lpstrFile = szFile;
                        ofn.nMaxFile = sizeof(szFile);
                        ofn.lpstrFilter = "PMX Files\0*.PMX\0FBX Files\0*.FBX\0All Files\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.lpstrFileTitle = nullptr;
                        ofn.nMaxFileTitle = 0;
                        ofn.lpstrInitialDir = nullptr;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                        if (GetOpenFileName(&ofn) == TRUE) {
                            // 用户选择了一个文件，路径在 ofn.lpstrFile 中
                            modelTransformManager::get()->addPmxModel(ofn.lpstrFile);
                            modelTransformManager::get()->setLoadPmxModel();
                            printf("select model successfully: %s\n", ofn.lpstrFile);
                        } else {
                            // 可以通过 CommDlgExtendedError() 获取更多错误信息
                            DWORD dwError = CommDlgExtendedError();
                            if (dwError == 0) {
                                puts("cancel");
                            } else {
                                printf("error: %lu\n", dwError);
                            }
                        }
#else
                        nfdchar_t *outPath{};
                        nfdresult_t nfdresult = NFD_OpenDialog("pmx;fbx", nullptr, &outPath);

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
#endif
                    }

                    if (ImGui::MenuItem("load model")){
                        nfdchar_t *outPath{};
                        nfdresult_t nfdresult = NFD_OpenDialog("pmx,fbx,glb,obj", nullptr, &outPath);

                        if (nfdresult == NFD_OKAY) {
                            modelManager::get()->updatePath(outPath);
                            printf("select model successfully: %s\n", outPath);

                            free(outPath);
                        } else if (nfdresult == NFD_CANCEL) {
                            puts("cancel");
                        } else {
                            printf("error: %s\n", NFD_GetError());
                        }
                    }

                    if (ImGui::MenuItem("Base Settings")){
                        //ImGui::SetNextWindowPos(ImVec2(baseSetting::GetWidth() / 2, baseSetting::GetHeight() / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

                        mBaseSettings = true;


                    }

                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});
                    if (ImGui::MenuItem("##e")){}
                    ImGui::PopStyleVar();
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Model")){
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});
                    if (ImGui::MenuItem("##e")){}
                    ImGui::PopStyleVar();

                    if (ImGui::MenuItem("Render")){
                        vkImguiRenderModel::get()->setRenderCurrentModel(eRenderModel);
                    }

                    if (ImGui::MenuItem("Editor")){
                        vkImguiRenderModel::get()->setRenderCurrentModel(eEditorModel);
                    }


                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});
                    if (ImGui::MenuItem("##e")){}
                    ImGui::PopStyleVar();
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            ImGuiIO& io = ImGui::GetIO();
            bool keyCtrl = io.KeyCtrl; // 检查 Ctrl 键是否被按下
            bool key_S = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S));
            if (keyCtrl && key_S){
                std::cout << "z" << std::endl;
                if (!modelTransformManager::get()->getInputModels().empty()){
                    auto j = js::vkProject::pmxToJson(modelTransformManager::get()->getInputModels().front());
                    js::vkProject::saveToUserSpecifiedLocation(j);
                } else {
                    auto j = nlohmann::json{};
                    js::vkProject::saveToUserSpecifiedLocation(j);
                }
            }


            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id);
        }
        ImGui::End();

    }

    void vkImgui::vkRenderWindow() {
        ImGuiStyle &xStyle = ImGui::GetStyle();
        xStyle.Colors[ImGuiCol_WindowBg].w = 0.0f;
        //ImGui::Begin("Main Render", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
        ImGui::Begin("Main Render", nullptr, ImGuiWindowFlags_NoBackground );
        {
            if (ImGui::IsMouseClicked(2) && vkImguiRenderModel::get()->getCurrentModelTask() == eEditorModel) {  // 2 中键 索引
                ImGui::OpenPopup("EditorPopup");
            }

            if (ImGui::BeginPopup("EditorPopup")) {
                if (ImGui::MenuItem("HighLight AABB")) {
                    vkImguiRenderModel::get()->setEditorCurrentModel(eHighLightAABB);
                }
                if (ImGui::MenuItem("Show All AABB")) {
                    vkImguiRenderModel::get()->setEditorCurrentModel(eAllAABB);
                }
                ImGui::EndPopup();
            }

        }
        ImGui::End();

        ImGuiStyle &yStyle = ImGui::GetStyle();
        yStyle.Colors[ImGuiCol_WindowBg].w = 1.f;
        yStyle.ItemSpacing = ImVec2(5.f, 10.f);
        //yStyle.FramePadding = ImVec2(0.f, 5.f);
        yStyle.FrameBorderSize = 1.f;
        ImGui::Begin("Obj Manager");
        {
            yStyle.FramePadding = ImVec2(0.f, 5.f);
            ImGui::Spacing();

            vkImguiManager::get()->getImguiTask(eObjectManager).renderCollapsingHeader();
        }
        ImGui::End();


        yStyle = ImGui::GetStyle();
        yStyle.Colors[ImGuiCol_WindowBg].w = 1.f;
        yStyle.ItemSpacing = ImVec2(5.f, 10.f);
        if (mDefaultWindow){
            ImGui::SetNextWindowFocus();
            mDefaultWindow = false;
        }
        ImGui::Begin("Character Manager");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25.f);
        {
            if (modelTransformManager::get()->isRenderPmxModel()) {

                yStyle.FramePadding = ImVec2(0.f, 5.f);

                if (ImGui::CollapsingHeader("Character", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::BeginChild("##0");
                    ImGui::Indent(20.f);
             //       ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

                    auto currentPos = modelTransformManager::get()->getPos();
                    auto currentScale = modelTransformManager::get()->getScale();
                    ImGui::SliderFloat3("Translation", &currentPos.x, -10.0f, 10.0f);
                    ImGui::SliderFloat("Scale", &currentScale, 0.1f, 5.f);
                    modelTransformManager::get()->setPosition(currentPos);
                    modelTransformManager::get()->setScale(currentScale);

                    if (ImGui::Button("Load vmd Anim", ImVec2(100, 20))) {
                        nfdchar_t *outPath{};
                        nfdresult_t nfdresult;
                        nfdresult = NFD_OpenDialog("vmd", nullptr, &outPath);
                        if (nfdresult == NFD_OKAY) {
                            modelTransformManager::get()->setLoadPmxModel();
                            modelTransformManager::get()->addPmxVMDAnim(outPath);
                            printf("select vmd successfully: %s\n", outPath);

                            free(outPath);
                        } else if (nfdresult == NFD_CANCEL) {
                            puts("cancel");
                        } else {
                            printf("error: %s\n", NFD_GetError());
                        }
                    }

                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_2))) {
                        nfdchar_t *outPath{};
                        nfdresult_t nfdresult;
                        nfdresult = NFD_OpenDialog("vmd", nullptr, &outPath);
                        if (nfdresult == NFD_OKAY) {
                            modelTransformManager::get()->setLoadPmxModel();
                            modelTransformManager::get()->addPmxVMDAnim(outPath);
                            printf("select vmd successfully: %s\n", outPath);

                            free(outPath);
                        } else if (nfdresult == NFD_CANCEL) {
                            puts("cancel");
                        } else {
                            printf("error: %s\n", NFD_GetError());
                        }
                    }


                    vkImguiManager::get()->getImguiTask(eToolbar).execute();

                  //  ImGui::PopStyleVar();
                    ImGui::EndChild();
                }


            }

        }
        ImGui::End();

        ImGui::Begin("Scene Effect");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25.f);
        {
            vkImguiManager::get()->getImguiTask(eTest).execute();
        }
        ImGui::End();


    }


    void vkImgui::RenderViewWindow(vk::DescriptorSet set, ImVec2 imageSize) {
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg] = ImVec4{0.15f, 0.15f, 0.21f, 1.f};

        ImGui::Begin("Scene Struct");
        {
            ImGui::Indent(10.f);

//            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // 白色文本
//            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.8f, 0.5f)); // 深蓝色标题背景
//            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.9f, 0.8f)); // 悬停时的颜色
//            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
//            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 20.f));
//            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 20.f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // 设置边框大小
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 8.0f));

            if (ImGui::TreeNodeEx("Root", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding)){
//                for(const auto& obj : objs){
                    if (ImGui::TreeNodeEx("Character", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding)){

                        ImGui::Text("Nilou");

                        ImGui::TreePop();
                    }
                if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding)) {


                    ImGui::TreePop();
                }
//                }
                ImGui::TreePop();
            }

            ImGui::PopStyleVar(2);

            //ImGui::PopStyleColor(4);
                       // ImGui::Text(obj.c_str());

        }
        ImGui::End();

        style.Colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
        ImGui::Begin("View");
        {
//            if (set != nullptr)
//                ImGui::Image((ImTextureID) set, imageSize);
            vkImguiManager::get()->getImguiTask(eView).execute();
        }
        ImGui::End();

        ImGui::Begin("Shadow Map");
        {
            vkImguiManager::get()->getImguiTask(eShadowMap).execute();
        }
        ImGui::End();

        if (mBaseSettings) {
//            ImGui::SetNextWindowPos({((float) baseSetting::GetWidth() - (float) baseSetting::GetWidth() / 2) * 0.5,
//                                     ((float) baseSetting::GetHeight() - (float) baseSetting::GetHeight() / 2) * 0.5});
            ImGui::SetNextWindowPos(ImVec2{static_cast<float>((baseSetting::GetWidth() - baseSetting::GetWidth() / 2) * 0.5),
                                           static_cast<float>((baseSetting::GetHeight() -  baseSetting::GetHeight() / 2) * 0.5)});
            ImGui::SetNextWindowSize(ImVec2((float )baseSetting::GetWidth() / 2, (float)baseSetting::GetHeight() / 2));
            //ImGui::SetNextWindowFocus();
            ImGui::Begin("Settings"); // Use ImGuiWindowFlags_AlwaysOnTop to keep it above other windows

            baseSetting::Gui();

            if (ImGui::Button("Cancel")){
                mBaseSettings = false;
            }
            ImGui::SameLine();
            if(ImGui::Button("Save")){
                baseSetting::save(R"(BaseSetting.json)");
                mBaseSettings = false;
                baseSetting::reStartApp();
            }

            ImGui::End();

            //bool demo = true;
            //ImGui::ShowDemoWindow(&demo);
        }

        ImGuiTerminal::get()->draw();
    }

} // yic















//            if (ImGui::BeginTabBar("My Tabs")) {
//                if (ImGui::BeginTabItem("Tab 1")) {
//                    ImGui::Text("Content of Tab 1");
//                    ImGui::EndTabItem();
//                }
//                if (ImGui::BeginTabItem("Tab 2")) {
//                    ImGui::Text("Content of Tab 2");
//                    ImGui::EndTabItem();
//                }
//                ImGui::EndTabBar();
//            }

//            if (ImGui::Button("Open Popup")) {
//                ImGui::OpenPopup("MyPopup");
//            }
//
//// 弹出Popup，需要与OpenPopup使用同一个标签
//            if (ImGui::BeginPopup("MyPopup")) {
////                ImGui::Text("This is a popup");
////                ImGui::Separator();
//                if (ImGui::Button("Option 1")) {
//                    // 处理 Option 1
//                }
//                if (ImGui::Button("Option 2")) {
//                    // 处理 Option 2
//                }
//                ImGui::EndPopup();
//            }

//            if (vkImguiRenderModel::get()->getCurrentModelTask() == eEditorModel){
//                if (ImGui::BeginPopupContextWindow("Item Context Menu")) {
//                    if (ImGui::MenuItem("HighLight AABB")) {
//                        vkImguiRenderModel::get()->setEditorCurrentModel(eHighLightAABB);
//                    }
//                    if (ImGui::MenuItem("Show All AABB")) {
//                        vkImguiRenderModel::get()->setEditorCurrentModel(eAllAABB);
//                    }
//                    ImGui::EndPopup();
//                }
//            }






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