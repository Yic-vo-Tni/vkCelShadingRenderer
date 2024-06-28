//
// Created by lenovo on 4/24/2024.
//

#ifndef VKMMD_DIRECTIONALLIGHT_H
#define VKMMD_DIRECTIONALLIGHT_H

#include "illuminant/lightBase.h"

#include "ke_q/vk_allocator.h"
#include "niLou/vkCamera.h"

namespace yic {

    class directionalLight : public lightBase{
    public:
        directionalLight();

        void update() override;

        [[nodiscard]] inline auto& getDirectionLightDir() const { return eye;}
        glm::vec3 getLightDirect() override{ return eye; }
        glm::vec3 getLightColor() override { return directLightColor; }

    private:
        void initVpMatrix();
    private:
        float dis = 30.f;
        glm::vec3 eye = {10.f, 25.f, 40.f};
        float nearPlane = 0.1f;
        float farPlane = 60.f;

        glm::vec3 directLightColor{1.1f, 0.9f, 0.9f};
    };

} // yic

#endif //VKMMD_DIRECTIONALLIGHT_H
