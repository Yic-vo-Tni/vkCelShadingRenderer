//
// Created by lenovo on 11/25/2023.
//

#ifndef VULKAN_LOG_H
#define VULKAN_LOG_H

#include "include/spdlog/spdlog.h"
#include "include/spdlog/sinks/stdout_color_sinks.h"
#include "include/spdlog/fmt/ostr.h"
#include "include/spdlog/sinks/ansicolor_sink.h"

#include "include/spdlog/sinks/base_sink.h"
#include "include/spdlog/details/null_mutex.h"

#include "nonCopyable.h"


namespace yic {

    class Log : public nonCopyable{
    public:
        static void init();
        static void addSinks(spdlog::sink_ptr sink) { mSinks.emplace_back(sink); }

        inline static std::shared_ptr<spdlog::logger> &getLogger() { return logger_; }
        inline static std::shared_ptr<spdlog::logger> &getCustomLogger() { return customLogger_; }

    private:
        static std::shared_ptr<spdlog::logger> logger_;
        static std::shared_ptr<spdlog::logger> customLogger_;

        static std::vector<spdlog::sink_ptr> mSinks;
    };


} // yic



#define vkTrance(...)    ::yic::Log::getLogger()->trace(__VA_ARGS__)
#define vkInfo(...)      ::yic::Log::getLogger()->info(__VA_ARGS__)
#define vkWarn(...)      ::yic::Log::getLogger()->warn(__VA_ARGS__)
#define vkError(...)     ::yic::Log::getLogger()->error(__VA_ARGS__)
#define vkFATAL(...)     ::yic::Log::getLogger()->fatal(__VA_ARGS__)
#define vkPink(...)      ::yic::Log::getCustomLogger()->info(__VA_ARGS__)


inline void vkCreate(const std::function<void()>& fun, const std::string& description, int n){
    try {
        fun();
    } catch (const vk::SystemError& e){
        if (enableDebug){
            vkError("failed to {0}", description);
        }
        exit(EXIT_FAILURE);
    }

    if (enableDebug){
        switch (n) {
            case -1:
                vkInfo("{0}, successfully", description);
                break;
            case 1:
                vkWarn("{0} successfully", description);
                break;
            case 2:
                vkError("{0} successfully", description);
                break;
            default:
                vkTrance("{0} successfully", description);
        }
    }
}


#endif //VULKAN_LOG_H
