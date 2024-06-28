//
// Created by lenovo on 4/24/2024.
//

#include "directionalLight.h"

namespace yic {

    directionalLight::directionalLight() {
        mLightState.type = eDirectionLight;

        mLightState.position = glm::vec3 {0.f};
        mLightState.direction = glm::vec3 {0.f, -1.f, 0.f};

        mLightState.constantAttenuation = 0.f;
        mLightState.linearAttenuation = 0.f;
        mLightState.quadraticAttenuation = 0.f;

        mVpUniformBuf = allocManager::build::bufSptr(sizeof (mCamera), vk::BufferUsageFlagBits::eUniformBuffer);
        initVpMatrix();
        mVpUniformBuf->updateBuffer(mCamera);
    }

    void directionalLight::initVpMatrix() {
        auto view = glm::lookAt(eye, glm::vec3 {0.f, 0.f, 0.f}, glm::vec3 {0.f, 1.f, 0.f});
        auto left = -dis, right = dis, bottom = -dis, top = dis;
        auto proj = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
        proj[1][1] *= -1;

        mCamera.vp = proj * view;

        vkImguiManager::get()->addRenderToTest([&](){
            if (ImGui::CollapsingHeader("Direction light", ImGuiTreeNodeFlags_DefaultOpen)) {

                ImGui::SliderFloat("distance", &dis, 0.f, 100.f);
                ImGui::SliderFloat("near plane", &nearPlane, -50.f, 50.f);
                ImGui::SliderFloat("far plane", &farPlane, 0.f, 200.f);
                ImGui::SliderFloat3("eye", &eye[0], -180.f, 180.f);

                ImGui::ColorPicker3("Direction Light", &directLightColor[0], ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_HDR);
            }
        });
    }


    void directionalLight::update() {
        auto view = glm::lookAt(eye, glm::vec3 {0.f, 0.f, 0.f}, glm::vec3 {0.f, 1.f, 0.f});
        auto left = -dis, right = dis, bottom = -dis, top = dis;
        auto proj = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
        proj[1][1] *= -1;

        mCamera.vp = proj * view;

        mVpUniformBuf->updateBuffer(mCamera);
    }


} // yic