//
// Created by lenovo on 11/15/2023.
//

#include "src/vk_App.h"

#ifdef  USE_WINMAIN

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {

#else
int main(){

#endif

    yic::Application app{};

    try {
        app.run();
    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


