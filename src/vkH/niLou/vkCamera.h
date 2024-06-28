//
// Created by lenovo on 12/28/2023.
//

#ifndef VULKAN_VKCAMERA_H
#define VULKAN_VKCAMERA_H

#include "log/Log.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

namespace yic {

    inline void printMatrixSpdlog(const glm::mat4& mat) {
        vkInfo("Printing Matrix:");
        for (int i = 0; i < 4; i++) {
            vkWarn("{: >8} {: >8} {: >8} {: >8}", mat[0][i], mat[1][i], mat[2][i], mat[3][i]);
        }
    }

    class vkCamera {
        float lastX =  1200.0f / 2.0;
        float lastY =  800.0 / 2.0;
        float Yaw_t = -90.0f;
        float pitch_t = 0.0f;

        glm::quat orientation;
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 4.0f);
        glm::mat4 mView{};
        glm::mat4 mProj{};

        float fov   =  45.0f;
        float sensitivity = 0.1f;
        //vkCamera() : position(0.5f, 0.f, 2.f), orientation(glm::quat(1., 0., 0., 0.)) {}
        vkCamera() : position(5.f, 10.f, 25.f), orientation(glm::quat(1., 0., 0., 0.)) {}
        friend Singleton<vkCamera>;
    public:
        glm::vec3 position;
        bool firstMouse = true;
        float dynamicSpeed = 1.f;
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        vkGet auto get = [](){ return Singleton<vkCamera>::get(); };
        [[nodiscard]] inline auto& getCameraPos() const { return cameraPos  ;}
        [[nodiscard]] inline auto& getPos() const { return position;}

        [[nodiscard]] glm::mat4 getViewMatrix() {
            mView = glm::translate(glm::mat4_cast(orientation), -position);
            return mView;
        }

        [[nodiscard]] glm::mat4 getProjMatrix(vk::Extent2D extent) {
            mProj = glm::perspective(fov, static_cast<float>(extent.width) / static_cast<float>(extent.height),
                                     0.1f, 500.f) * glm::scale(glm::mat4(1.f), glm::vec3(1.f, -1.f, 1.f));

            return mProj;
        }

        [[nodiscard]] glm::mat4 getVPMatrix(vk::Extent2D extent) {
            return glm::mat4{ getProjMatrix(extent) * getViewMatrix()};
        }

//        void updateCamera(vkAllocator* buffer, vk::Extent2D extent){
//            buffer->updateBuffer(getVPMatrix(extent));
//        }

        void mouseCallback(double xPos_d, double yPos_d) {
            auto xPos = static_cast<float>(xPos_d);
            auto yPos = static_cast<float>(yPos_d);
            if (firstMouse){
                lastX = xPos;
                lastY = yPos;
                firstMouse = false;
            }
            float xOffset = xPos - lastX;
            //float yOffset = yPos - lastY;
            float yOffset = lastY - yPos;
            lastX = xPos;
            lastY = yPos;

            xOffset *= sensitivity;
            yOffset *= sensitivity;

            Yaw_t += xOffset;
            pitch_t += yOffset;
            rotate(xOffset, yOffset);
            updateCameraFront(Yaw_t, pitch_t);
        }

        void scrollCallback(double xOffset, double yOffset) {
            if (yOffset > 0.0) {
                dynamicSpeed *= 1.2f;
            } else if (yOffset < 0.0) {
                dynamicSpeed *= 0.8f;
            }
        }

    private:
        void rotate(float yaw, float pitch) {
            glm::vec3 localUp = glm::rotate(orientation, glm::vec3(0.f, 1.f, 0.f));
            glm::quat mPitch = glm::angleAxis(glm::radians(-pitch), glm::vec3(1.f, 0.f, 0.f));
            glm::quat mYaw = glm::angleAxis(glm::radians(yaw), localUp);
            orientation = mPitch * mYaw * orientation;
            orientation = glm::normalize(orientation);
        }

        void updateCameraFront(float yaw, float pitch) {
            glm::vec3 front;
            front.x = float(cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
            front.y = float(sin(glm::radians(pitch)));
            front.z = float(sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
            cameraFront = glm::normalize(front);
        }

    };
} // yic

#endif //VULKAN_VKCAMERA_H





//glm::vec3 position;
//float fov   =  45.0f;
//bool firstMouse = true;
//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
//glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
//vkGet auto get = [](){ return Singleton<vkCamera>::get(); };
//
//[[nodiscard]] glm::mat4 getViewMatrix() {
//    mView = glm::translate(glm::mat4_cast(orientation), -position);
//    return mView;
//}
//
//[[nodiscard]] glm::mat4 getProjMatrix(vk::Extent2D extent) {
//    mProj = glm::perspective(fov, static_cast<float>(extent.width) / static_cast<float>(extent.height),
//                             0.1f, 100.f) * glm::scale(glm::mat4(1.f), glm::vec3(1.f, -1.f, 1.f));
//    return mProj;
//}
//
//[[nodiscard]] glm::mat4 getVPMatrix(vk::Extent2D extent) {
//    return glm::mat4{ getProjMatrix(extent) * getViewMatrix()};
//}
//
//void updateCamera(vkAllocator* buffer, vk::Extent2D extent){
//    buffer->updateBuffer(getVPMatrix(extent));
//}
//
//void mouseCallback(double xPos_d, double yPos_d) {
//    auto xPos = static_cast<float>(xPos_d);
//    auto yPos = static_cast<float>(yPos_d);
//    if (firstMouse){
//        lastX = xPos;
//        lastY = yPos;
//        firstMouse = false;
//    }
//    float xOffset = xPos - lastX;
//    float yOffset = lastY - yPos;
//    lastX = xPos;
//    lastY = yPos;
//
//    float sensitivity = 0.1f;
//    xOffset *= sensitivity;
//    yOffset *= sensitivity;
//
//    Yaw_t += xOffset;
//    pitch_t += yOffset;
//    rotate(xOffset, yOffset);
//    updateCameraFront(Yaw_t, pitch_t);
//}
//
//void scrollCallback(double xOffset, double yOffset) {
//    fov -= (float)yOffset;
//    if (fov < 1.0f)
//        fov = 1.0f;
//    if (fov > 45.0f)
//        fov = 45.0f;
//}
//
//private:
//void rotate(float yaw, float pitch) {
//    glm::vec3 localUp = glm::rotate(orientation, glm::vec3(0.f, 1.f, 0.f));
//    glm::quat mPitch = glm::angleAxis(glm::radians(-pitch), glm::vec3(1.f, 0.f, 0.f));
//    glm::quat mYaw = glm::angleAxis(glm::radians(yaw), localUp);
//    orientation = mPitch * mYaw * orientation;
//    orientation = glm::normalize(orientation);
//}
//
//void updateCameraFront(float yaw, float pitch) {
//    glm::vec3 front;
//    front.x = float(cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
//    front.y = float(sin(glm::radians(pitch)));
//    front.z = float(sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
//    cameraFront = glm::normalize(front);
//}

























//void move(float forwardBackward, float leftRight) {
//            glm::vec3 globalForward(0., 0., -1);
//            glm::vec3 globalRight(1., 0., 0.);
//            glm::vec3 forward = glm::normalize(glm::rotate(orientation, globalForward));
//            glm::vec3 right = glm::normalize(glm::rotate(orientation, globalRight));
//
//            position += forward * forwardBackward + right * leftRight;
//        }