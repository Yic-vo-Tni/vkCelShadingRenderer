//
// Created by lenovo on 2/9/2024.
//

#ifndef VKMMD_VKIDGENERATOR_H
#define VKMMD_VKIDGENERATOR_H

namespace vkH {

    class vkIdGenerator {
    public:
        vkGet auto get = [](){ return yic::Singleton<vkIdGenerator>::get(); };

        std::string generatorUniqueId(){
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distribution(0, 9999);

            std::stringstream ss;
            ss << millis << "-" << distribution(gen);

            return ss.str();
        }
    };

} // vkH

#endif //VKMMD_VKIDGENERATOR_H
