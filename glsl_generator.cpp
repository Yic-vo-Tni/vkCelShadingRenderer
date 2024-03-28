//
// Created by lenovo on 3/26/2024.
//
#include "src/shaders/vot/genGlslBase.h"
#include "glm/glm.hpp"

int main(){

//    vot::glslGenerator generator;
//
//    try {
//        generator.run();
//    } catch (std::exception& e){
//        std::cerr << e.what() << std::endl;
//        return -1;
//    }
//
//    return 0;

    vot::vkGLSLGenerator generator{};

    vot::vec2 inUV, outUV;
    vot::vec3 inPos, inNor, outPos, outNor;
    generator.addInput(0, inPos)
            .addInput(1, inNor)
            .addInput(2, inUV);
    generator.addOutput(0, outPos)
            .addOutput(1, outNor)
            .addOutput(2, outUV);


    const char* x = R"glsl(
    #version 460;
layout(location = 0)in vec3 aPos;
)glsl";


    try {
        generator.saveToFile(shaderPath "vktest/mmd.vert");
    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}