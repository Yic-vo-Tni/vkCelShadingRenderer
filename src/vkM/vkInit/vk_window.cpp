//
// Created by lenovo on 11/25/2023.
//

#include "vk_window.h"

#include <utility>

namespace yic {

    Window::Window(int w, int h, std::string  name) : width_(w), height_(h), name_(std::move(name)){
        Log::init();
        initWindow();
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

    void Window::defaultMouseCallback(GLFWwindow *window, double xPos, double yPos) {}

    void Window::defaultScrollCallback(GLFWwindow *window, double xOffset, double yOffset) {}

    void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    }

} // yic