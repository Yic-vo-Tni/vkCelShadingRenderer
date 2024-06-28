//
// Created by lenovo on 5/8/2024.
//

#include "baseSetting.h"

namespace yic {

    void baseSetting::load(const std::string &fileName) {
        std::ifstream i(fileName);
        json j;
        i >> j;
        get()->mWidth = j.at(jsonFields::width).get<int>();
        get()->mHeight = j.at(jsonFields::height).get<int>();
        get()->mOptions_window.itemCurrentIdx = j.at(jsonFields::window_size_id).get<int>();
        get()->mPrimitiveTopology = j.at(jsonFields::drawMode).get<vk::PrimitiveTopology>();
    }

    void baseSetting::save(const std::string &fileName) {
        json j;
        j[jsonFields::width] = get()->mWidth;
        j[jsonFields::height] = get()->mHeight;
        j[jsonFields::window_size_id] = get()->mOptions_window.itemCurrentIdx;
        j[jsonFields::drawMode] = get()->mPrimitiveTopology;

        std::ofstream o(fileName);
        o << j.dump(4);
    }

    void baseSetting::Gui() {
        auto& id = get()->mOptions_window.itemCurrentIdx;
        if (ImGui::Combo("Window Size", &id,
                         get()->mOptions_window.items.data(), (int )get()->mOptions_window.items.size())){
            if (get()->mOptions_window.itemCurrentIdx == 0){
                get()->mWidth = 1200;
                vkWarn(get()->mWidth);
                get()->mHeight = 800;
                vkWarn(get()->mHeight);
            }
            if (get()->mOptions_window.itemCurrentIdx == 1){
                get()->mWidth = 1600;
                get()->mHeight = 1200;
                vkInfo(get()->mHeight);
            }
            if (get()->mOptions_window.itemCurrentIdx == 2){
                get()->mWidth = 2000;
                get()->mHeight = 1400;
                vkError(get()->mHeight);
            }
        }

        auto& ptId = get()->mOptions_PrimitiveTopology.itemCurrentIdx;
        if (ImGui::Combo("Primitive Topology", &ptId,
                         get()->mOptions_PrimitiveTopology.items.data(), (int)get()->mOptions_PrimitiveTopology.items.size())){
            if (ptId == 0){
                get()->mPrimitiveTopology = vk::PrimitiveTopology::eTriangleList;
            }
            if (ptId == 1){
                get()->mPrimitiveTopology = vk::PrimitiveTopology::ePointList;
            }
            if (ptId == 2){
                get()->mPrimitiveTopology = vk::PrimitiveTopology::eLineList;
            }
        }

    }

    void baseSetting::firstDefaultValue(const std::string &fileName) {
        std::ifstream i(fileName);
        json j;
        bool empty = true;

        if (i.good()){
            try {
                i >> j;
                empty = j.empty();
            } catch (json::parse_error &e){
                empty = true;
            }
        }

        if (empty){
            save(fileName);
        }
    }

    void baseSetting::reStartApp() {
//        char exePath[MAX_PATH];
//        if(GetModuleFileNameA(nullptr, exePath, MAX_PATH) == 0){
//            vkError("failed to get module file name");
//            return;
//        }
//
//        STARTUPINFOA si = {sizeof (si)};
//        PROCESS_INFORMATION pi;
//        if (!CreateProcessA(exePath, nullptr, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi)){
//            vkError("failed to create process");
//        }
//
//        CloseHandle(pi.hProcess);
//        CloseHandle(pi.hThread);

        std::system("start vulkan.exe");

        exit(0);
    }

} // yic