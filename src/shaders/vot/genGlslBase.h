//
// Created by lenovo on 3/26/2024.
//

#ifndef VKMMD_GENGLSLBASE_H
#define VKMMD_GENGLSLBASE_H

#include <cstdint>
#include "fstream"
#include "string"
#include "iostream"
#include "vector"
#include "filesystem"
#include "config.h"
#include "customMath.h"

namespace vot {

    enum shaderType{
        eVertex, eFragment,
        eRgen, eRchit, eRmiss,
    };

    using vec2 = cVec2;
    using vec3 = cVec3;

//    struct Type{
//        vec2 ;
//        vec3 ;
//    };

    template<class T>
    inline std::string typeToString() { return "unknown type";}

    template<>
    inline std::string typeToString<vec2>() {
        return "vec2";
    }
    template<>
    inline std::string typeToString<vec3>() {
        return "vec3";
    }

    class vkGLSLGenerator{
    public:
        explicit vkGLSLGenerator(const uint32_t version = 460){
            addLine("#version " + std::to_string(version));
        }

        template<class T>
        vkGLSLGenerator& addInput(const int& location, const T& type_name){
            std::string type = typeToString<T>();
            std::string var = type_name.getID();
            addLine("layout (location = " + std::to_string(location) + ") in " + type + " " + var + ";");
            return *this;
        }
        template<class T>
        vkGLSLGenerator& addOutput(const int& location, const T& type_name){
            std::string type = typeToString<T>();
            std::string var = type_name.getID();
            addLine("layout (location = " + std::to_string(location) + ") in " + type + " " + var + ";");
            return *this;
        }
//        vkGLSLGenerator& addOutput(const int& location, const Type& type, const std::string & name){
//            addLine("layout (location = " + std::to_string(location) + ") out " + typeToString(type) + " " + name + ";");
//            return *this;
//        }

    public:
        void addLine(const std::string & line){
            mShaderCode.emplace_back(line);
        }

//        std::string typeToString(Type& typeName) {
//            switch (typeName) {
//                case typename:
//            }
//            return "vec3 " + typeName.getID() + ";";
////            switch (type) {
////                case Type::eVec3: return "vec3";
////                case Type::eVec2: return "vec2";
////                case Type::eMat4: return "mat4";
////                default: return "void"; // 或者你可以选择抛出一个异常
////            }
//        }

    public:
        void saveToFile(const std::string& fileName){
            std::filesystem::path filePath(fileName);
            auto directory = filePath.parent_path();

            if (!directory.empty() && !std::filesystem::exists(directory)) {
                std::filesystem::create_directories(directory);
            }

            std::ofstream file(fileName);
            if (!file.is_open()){
                std::cerr << " unable to open file for writing" << fileName << std::endl;
            }

            for(const auto& line : mShaderCode){
                file << line << std::endl;
            }
            file.close();
            std::cout << "Shader saved to " << fileName << std::endl;
        }

    private:
        std::vector<std::string> mShaderCode;

    };


} // vot

#endif //VKMMD_GENGLSLBASE_H





