//
// Created by lenovo on 11/25/2023.
//

#ifndef VULKAN_VK_WINDOW_H
#define VULKAN_VK_WINDOW_H

#include "log/Log.h"
#include "niLou/vkCamera.h"

namespace yic {

    class Window : public nonCopyable{
        explicit Window(int w = {}, int h = {}, std::string  name = {});
        ~Window();

        friend Singleton<Window>;
    public:
        vkGet auto get = [](int w = {}, int h = {}, const std::string& name = {}){ return Singleton<Window>::get(w, h, name); };

        [[nodiscard]] auto& window() const { return window_;}

        void processInput();

        [[nodiscard]] auto& width() const { return width_;}
        [[nodiscard]] auto& height() const { return height_;}
    private:

        int width_, height_;
        std::string name_;
        GLFWwindow* window_{};

        void initWindow();

    public:
        bool firstClick = true;
        double xLast{}, yLast{};
        float deltaTime{}, lastFrame{};

        void calculateTime(){
            auto currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
        };
        void glfwCallback();
    private:
        static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
        static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
        static void defaultMouseCallback(GLFWwindow* window, double xPos, double yPos);
        static void defaultScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void keyCallback(GLFWwindow* window, int key, int scancode,int action, int mode);
    };

} // yic

#endif //VULKAN_VK_WINDOW_H
