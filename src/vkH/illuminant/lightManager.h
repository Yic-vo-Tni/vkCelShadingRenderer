//
// Created by lenovo on 4/24/2024.
//

#ifndef VKMMD_LIGHTMANAGER_H
#define VKMMD_LIGHTMANAGER_H

#include "illuminant/directionalLight.h"

namespace yic {

    class lightManager {
    public:
        lightManager();
        vkGet auto get = [](){ return Singleton<lightManager>::get();};

        [[nodiscard]] inline static auto& getDirectionLight() { return get()->mDirectionLight;}
        void addDirectionalLight();

        void update(){
            for(auto& light : mLight){
                light->update();
            }
        }
    private:
        std::shared_ptr<lightBase> mDirectionLight;
        std::vector<std::shared_ptr<lightBase>> mLight;

    };

} // yic

#endif //VKMMD_LIGHTMANAGER_H
