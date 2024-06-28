//
// Created by lenovo on 3/29/2024.
//

#ifndef VKMMD_VKRESMANAGER_H
#define VKMMD_VKRESMANAGER_H

#include <typeindex>
#include "vkResourceBase.h"

namespace rs {

    class vkResManager {
    private:
        // 使用 type_index 作为键来区分不同类型的资源
        std::unordered_map<std::type_index, std::vector<std::shared_ptr<vkResourceBase>>> resources;

    public:
        template <typename T, typename... Args>
        void loadResource(Args&&... args) {
            auto res = std::make_shared<T>(std::forward<Args>(args)...);
            resources[typeid(T)].push_back(res);
        }

        template <typename T>
        std::vector<std::shared_ptr<T>> getResources() {
            std::vector<std::shared_ptr<T>> typedResources;
            auto& baseResources = resources[typeid(T)];
            for (auto& baseRes : baseResources) {
                typedResources.push_back(std::static_pointer_cast<T>(baseRes));
            }
            return typedResources;
        }
    };

} // rs

#endif //VKMMD_VKRESMANAGER_H
