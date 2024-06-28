//
// Created by lenovo on 3/28/2024.
//

#include "vkProject.h"

namespace js {

//

    void vkProject::saveToUserSpecifiedLocation(const nlohmann::json &save) {
        nfdchar_t *savePath = nullptr;
        nfdresult_t r = NFD_SaveDialog("vot", nullptr, &savePath);

        if (r == NFD_OKAY){
            std::string filePath{savePath};

            if (filePath.substr(filePath.length() - 4, 4) != "vot"){
                filePath += ".vot";
            }

            std::ofstream file(filePath);
            if (file.is_open()){
                file << save.dump(4);
                file.close();
                std::cout << "project saved to " << filePath << std::endl;
            } else{
                std::cerr << "failed to open file for writing : " << filePath << std::endl;
            }

        } else if(r == NFD_CANCEL){
            std::cout << "canceled. " << std::endl;
        } else{
            std::cerr << "Error: " << NFD_GetError() << std::endl;
        }
    }

    nlohmann::json vkProject::loadToUserSpecifiedLocation() {
        nfdchar_t *savePath = nullptr;
        nfdresult_t r = NFD_OpenDialog("vot", nullptr, &savePath);
        nlohmann::json j;

        if (r == NFD_OKAY){
            std::string filePath{savePath};

            std::ifstream file(filePath);
            if (file.is_open()){
                file >> j;
                file.close();
                std::cout << "project load from " << filePath << std::endl;
                return j;
            } else{
                std::cerr << "failed to open file for reading : " << filePath << std::endl;
            }

        } else if(r == NFD_CANCEL){
            std::cout << "canceled. " << std::endl;
        } else{
            std::cerr << "Error: " << NFD_GetError() << std::endl;
        }

        return {};
    }

} // nfd












//void vkProject::saveProject(const js::vkProject &project, const std::string &filePath) {
//        std::ofstream file(filePath);
//
//        if (!file.is_open()){
//            std::cerr << "failed to open file for : " << filePath << std::endl;
//            return;
//        }
//
//        nlohmann::json j = project.toJson();
//        file << j.dump(4);
//        file.close();
//    }
//
//
//    bool vkProject::loadProject(js::vkProject &project, const std::string &path) {
//        std::ifstream file(path);
//
//        if (!file.is_open()){
//            std::cerr << "failed to open file for :" << path << std::endl;
//            return false;
//        }
//
//        nlohmann::json j;
//        file >> j;
//        project.fromJson(j);
//        return true;
//    }