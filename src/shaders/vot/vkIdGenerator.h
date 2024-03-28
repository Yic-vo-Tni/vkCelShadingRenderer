//
// Created by lenovo on 2/9/2024.
//

#ifndef VKMMD_VKIDGENERATOR_H
#define VKMMD_VKIDGENERATOR_H


#include <utility>
#include <string>
#include "chrono"
#include "random"

namespace vot {

    template <typename T>
    class Singleton{
    public:
        Singleton() = default;
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

        template<typename...Args>
        static T* get(Args&&... args){
            static T singleton{std::forward<Args>(args)...};
            return &singleton;
        }
    };

    #define vkGet inline static constexpr

    class vkIdGenerator {
    public:
        vkGet auto get = [](){ return Singleton<vkIdGenerator>::get(); };

        std::string generatorUniqueId(){
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distribution(0, 9999);

            std::stringstream ss;
            ss << "v" << millis << "t" << distribution(gen);

            return ss.str();
        }
    };

} // vkH

#endif //VKMMD_VKIDGENERATOR_H
