//
// Created by lenovo on 11/25/2023.
//

#include "vk_App.h"

namespace yic {

    Application::Application() = default;

    Application::~Application() = default;

    void Application::run() {
        while (!glfwWindowShouldClose(Window::get()->window())){
            glfwPollEvents();
            if (vk_base::isMinimized())
                continue;

            Window::get()->processInput();
            rhi_->drawFrame();
        }
    }


} // yic













