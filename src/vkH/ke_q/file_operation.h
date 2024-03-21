//
// Created by lenovo on 12/20/2023.
//

#ifndef VULKAN_FILE_OPERATION_H

#include "yic_pch.h"

namespace ke_q{

//    inline std::string findFile(const std::string& inFilename, const std::vector<std::string>& directories, bool warn = false)
//    {
//        std::ifstream stream;
//
//        {
//            stream.open(inFilename.c_str());
//            if(stream.is_open())
//            {
//                return inFilename;
//            }
//        }
//
//        for(const auto& directory : directories)
//        {
//            std::string filename = directory + "/" + inFilename;
//            stream.open(filename.c_str());
//            if(stream.is_open())
//            {
//                return filename;
//            }
//        }
//
//        return {};
//    }

    inline std::string loadFile(const std::string& filename, bool binary = true)
    {
        std::string   result;
        std::ifstream stream(filename, std::ios::ate | (binary ? std::ios::binary : std::ios_base::openmode(0)));

        if(!stream.is_open())
        {
            return result;
        }

        result.reserve(stream.tellg());
        stream.seekg(0, std::ios::beg);

        result.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return result;
    }

//    inline std::string loadFile(const std::string& filename, bool binary)
//    {
//        return loadFile(filename, binary);
//    }
//
//
//    inline std::string loadFile(const std::string& filename, bool binary, const std::vector<std::string>& directories, std::string& filenameFound, bool warn = false){
//        filenameFound = findFile(filename, directories, warn);
//        if (filenameFound.empty()){
//            return {};
//        } else{
//            return loadFile(filenameFound, binary);
//        }
//    }
//
//    inline std::string loadFile(const std::string filename, bool binary, const std::vector<std::string>& directories, bool warn = false){
//        std::string fileNameFound;
//        return loadFile(filename, binary, directories, fileNameFound, warn);
//    }


}



#define VULKAN_FILE_OPERATION_H

#endif //VULKAN_FILE_OPERATION_H
