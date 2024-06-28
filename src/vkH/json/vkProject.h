//
// Created by lenovo on 3/28/2024.
//

#ifndef VKMMD_VKPROJECT_H
#define VKMMD_VKPROJECT_H

#include "nlohmann/json.hpp"
#include "nfd.h"
#include "pmx/pmx_header.h"

namespace js {

    class vkProject {
    public:
        [[nodiscard]] nlohmann::json toJson() const {
            return nlohmann::json {{"id", mId}, {"name", mName}, {"tags", mTags}};
        }

        void fromJson(const nlohmann::json& json){
            mId = json.at("id").get<int>();
            mName = json.at("name").get<std::string>();
            std::cout << mName << std::endl;
            mTags = json.at("tags").get<std::vector<std::string>>();
        }

        static void saveToUserSpecifiedLocation(const nlohmann::json& save);
        [[nodiscard]] static nlohmann::json loadToUserSpecifiedLocation();

        [[nodiscard]] static nlohmann::json pmxToJson(const vkPmx::Input& input) {
            if (input.m_modelPath.empty()){
                return {};
            }
            return nlohmann::json {{"pmx path", input.m_modelPath}, {"vmd path", input.m_vmdPaths}};
        }

        [[nodiscard]] static vkPmx::Input pmxFromJson(const nlohmann::json& json){
            return  vkPmx::Input{
                    json.at("pmx path").get<std::string>(),
                    json.at("vmd path").get<std::vector<std::string>>()
            };
        }


    public:
        int mId;
        std::string mName;
        std::vector<std::string> mTags;
        vkPmx::Input mInput;
    };

//    class test{
//    public:
//        vkProject project;
//
//        void run(){
//            if (ImGui::Button("save proj")) {
//                project.mId = 1;
//                project.mName = "my first proj";
//                project.mTags = {"graphics", "pro", "ldskjf", "dslfj"};
//                nlohmann::json proj = project.toJson();
//                js::vkProject::saveToUserSpecifiedLocation(proj);
//            }
//            if (ImGui::Button("load pro")){
//                nlohmann::json j = js::vkProject::loadToUserSpecifiedLocation();
//                if (!j.empty())
//                    project.fromJson(j);
//            }
//        }
//    };

} // nfd

#endif //VKMMD_VKPROJECT_H
