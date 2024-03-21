//
// Created by lenovo on 11/15/2023.
//

#include "src/vk_App.h"

int main(){
    yic::Application app{};

    try {
        app.run();
    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

