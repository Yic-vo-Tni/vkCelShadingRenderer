//
// Created by lenovo on 11/25/2023.
//

#include "vk_window.h"

#include <utility>

namespace yic {

    Window::Window(int w, int h, std::string  name) : width_(w), height_(h), name_(std::move(name)){
        Log::init();
        initWindow();
        setGLFWWindowIcon(window_);
    }

    Window::~Window() {
        glfwTerminate();
        glfwDestroyWindow(window_);
    }

    void Window::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window_ = glfwCreateWindow(width_, height_, name_.c_str(), nullptr, nullptr);
    }

    void Window::setGLFWWindowIcon(GLFWwindow *window) {
        int w, h, channels;
        unsigned char* imageData = stbi_load(texPath "ico/ico.png", &w, &h, &channels, 4);

        if (!imageData) return;
        GLFWimage ico{w, h, imageData};
        glfwSetWindowIcon(window_, 1, &ico);
    }

    glm::vec3 Window::detectionMouseRay() {
        double mouseX, mouseY;
        int width, height;
        glfwGetCursorPos(window_, &mouseX, &mouseY);
        glfwGetFramebufferSize(window_, &width, &height);

        float x = (float )(2.f * mouseX) / (float )width - 1.f;
        float y = (float )(2.f * mouseY) / (float )height - 1.f;

        glm::vec3 ray_ndc(x, y , 1.f);

        glm::vec4 ray_clip = glm::vec4 (ray_ndc.x, ray_ndc.y, -1.f, 1.f);
        glm::vec4 ray_eye = glm::inverse(vkCamera::get()->getProjMatrix(vk::Extent2D{(uint32_t )width, (uint32_t )height})) * ray_clip;
        ray_eye.z = -1.f;
        ray_eye.w = 0.f;

        glm::vec3 ray_wor = glm::inverse(vkCamera::get()->getViewMatrix()) * ray_eye;
        glm::vec3 ray_wor_nor = glm::normalize(glm::inverse(vkCamera::get()->getViewMatrix()) * ray_eye);

        return ray_wor_nor;
    }

    bool Window::rayIntersectsAABB(glm::vec3 min, glm::vec3 max, float &nearest) {
        auto ray_origin = vkCamera::get()->getPos();
        glm::vec3 invDir = 1.f / detectionMouseRay();

        float tmin = (min.x - ray_origin.x) * invDir.x;
        float tmax = (max.x - ray_origin.x) * invDir.x;

        if (tmin > tmax) std::swap(tmin, tmax);

        for (int i = 1; i < 3; ++i) {
            float t1 = (min[i] - ray_origin[i]) * invDir[i];
            float t2 = (max[i] - ray_origin[i]) * invDir[i];

            if (t1 > t2) std::swap(t1, t2);

            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);

            if (tmin > tmax || tmax < 0) return false;
        }

        if (tmin < tmax && tmin >= 0){
            nearest = tmin;
            return true;
        }
        return false;
    }

    bool Window::rayIntersectsXYZAABB(glm::vec3 min, glm::vec3 max) {
        auto ray_origin = vkCamera::get()->getPos();
        glm::vec3 invDir = 1.f / detectionMouseRay();

        float tmin = (min.x - ray_origin.x) * invDir.x;
        float tmax = (max.x - ray_origin.x) * invDir.x;

        if (tmin > tmax) std::swap(tmin, tmax);

        for (int i = 1; i < 3; ++i) {
            float t1 = (min[i] - ray_origin[i]) * invDir[i];
            float t2 = (max[i] - ray_origin[i]) * invDir[i];

            if (t1 > t2) std::swap(t1, t2);

            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);

            if (tmin > tmax || tmax < 0) return false;
        }

        if (tmin < tmax && tmin >= 0){
            return true;
        }
        return false;
    }

    void Window::processInput() {
        if(glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        }

        float cameraSpeed{static_cast<float>(5 * deltaTime) * vkCamera::get()->dynamicSpeed};

        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
            vkCamera::get()->position += cameraSpeed * vkCamera::get()->cameraFront;
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
            vkCamera::get()->position -= cameraSpeed * vkCamera::get()->cameraFront;
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
            vkCamera::get()->position -= glm::normalize(glm::cross(vkCamera::get()->cameraFront, vkCamera::get()->cameraUp)) * cameraSpeed;
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
            vkCamera::get()->position += glm::normalize(glm::cross(vkCamera::get()->cameraFront, vkCamera::get()->cameraUp)) * cameraSpeed;

//        if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window_, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS){
//            if (glfwGetKey(window_, GLFW_KEY_S)){
//                auto j = js::vkProject::pmxToJson(modelTransformManager::get()->getInputModels().front());
//                js::vkProject::saveToUserSpecifiedLocation(j);
//            }
//        }

        if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
            if (firstClick){
                glfwSetCursorPos(window_, xLast, yLast);
                firstClick = false;
            }
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPosCallback(window_, mouseCallback);
            glfwSetScrollCallback(window_, scrollCallback);

            glfwSetKeyCallback(window_, keyCallback);
        }
        if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE){
            glfwGetCursorPos(window_, &xLast, &yLast);
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPosCallback(window_, defaultMouseCallback);
            glfwSetScrollCallback(window_, defaultScrollCallback);
            glfwSetMouseButtonCallback(window_, mouseButtonCallback);
            firstClick = true;
            vkCamera::get()->firstMouse = true;

            glfwSetKeyCallback(window_, keyCallback);
            glfwSetDropCallback(window_, dropCallback);
            glfwSetCharCallback(window_, setCharCallback);
        }
    }

    void Window::glfwCallback() {
        glfwSetWindowUserPointer(window_, get());
    }

    void Window::mouseCallback(GLFWwindow *window, double xPos, double yPos) {
        vkCamera::get()->mouseCallback(xPos, yPos);
    }

    void Window::scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
        vkCamera::get()->scrollCallback(xOffset, yOffset);
    }

    void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
        ImGuiIO& io = ImGui::GetIO();
        if (action == GLFW_PRESS)
            io.KeysDown[key] = true;
        if (action == GLFW_RELEASE)
            io.KeysDown[key] = false;

        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mode);
    }

    void Window::dropCallback(GLFWwindow *window, int count, const char **paths) {
        if (count > 0){
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            for(auto& rec : get()->mImageRects){
                auto min = rec.second.min;
                auto max = rec.second.max;

                if (x >= min.x && x <= max.x && y >= min.y && y < max.y){
                    get()->mImgPath.first = rec.first;
                    get()->mImgPath.second = paths[0];
                    break;
                }
            }

        }
    }

    void Window::checkImgRect() {
        for(auto& rec : mImageRects){
            auto min = rec.second.min;
            auto max = rec.second.max;

            if(rectX >= min.x && rectX <= max.x && rectY >= min.y && rectY <= max.y){
                mImgPath.first = rec.first;
                break;
            }
        }
    }

    void Window::defaultMouseCallback(GLFWwindow *window, double xPos, double yPos) {
        if (get()->mMoveXYZ){

//            if (get()->isFirstMoveXYZ ){
//                get()->isFirstMoveXYZ = false;
//            } else {
//                double dx = xPos - get()->mLastX;
//                double dy = yPos - get()->mLastY;
//
//                get()->mLastX = xPos;
//                get()->mLastY = yPos;
//
//                float sensitivity = 0.02f;
//                if (dx >= 1 || dx <= -1) {
//                    get()->mMoveX = glm::vec3(dx, 0, 0);
//                    //get()->mMoveX = glm::vec3 (dx * sensitivity, 0, 0);
//                    get()->mMoveZ = glm::vec3(0, 0, -dx * sensitivity);
//                    get()->mMoveY = glm::vec3(0, -dy * sensitivity, 0);
//                }
//
//                std::cout << dx << std::endl;
//            }
//
//
//
//            get()->mLastX = xPos;
//            get()->mLastY = yPos;

           // std::cout << get()->MoveX().x << " " << get()->MoveY().y << " " << get()->MoveZ().z << std::endl;
        }

      //  vkInfo("{0}", get()->MoveX().x);
    }

    void Window::defaultScrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheelH += (float)xOffset;
        io.MouseWheel += (float)yOffset;
    }

    void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
            get()->intersect = true;

        } else{
            get()->intersect = false;
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
            get()->mMoveXYZ = true;
            get()->isFirstMoveXYZ = true;
            glfwGetCursorPos(window, &get()->mLastX, &get()->mLastY);
        } else if(action == GLFW_RELEASE){
            get()->mMoveXYZ = false;
        }
    }

    void Window::setCharCallback(GLFWwindow *window, unsigned int c) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantTextInput){
            io.AddInputCharacter(c);
        }
    }

    void Window::updateXYZMoveDistance() {
        static double lastX = 0, lastY = 0;
        double currentX, currentY;
        glfwGetCursorPos(window_, &currentX, &currentY);

        double dx = currentX - lastX;
        double dy = currentY - lastY;

        if (get()->mMoveXYZ) {
            if (dx == 0 && dy == 0) {
                get()->mMoveX = glm::vec3(0.0f);
                get()->mMoveY = glm::vec3(0.0f);
                get()->mMoveZ = glm::vec3(0.0f);
            } else {
                float sensitivity = 0.02f;
                get()->mMoveX = glm::vec3(dx * sensitivity, 0, 0);
                get()->mMoveY = glm::vec3(0, -dy * sensitivity, 0);
                get()->mMoveZ = glm::vec3(0, 0, -dx * sensitivity);
            }
        }

        lastX = currentX;
        lastY = currentY;
    }

} // yic



//            glfwGetCursorPos(window, &get()->rectX, &get()->rectY);
//
//            get()->mImgPath.second = paths[0];
//            get()->updateImgRect = true;
// pmx mesh


//            get()->checkImgRect();
//        } else {
//            get()->updateImgRect = false;