//
// Created by lenovo on 11/25/2023.
//

#include "Log.h"

namespace yic {

    std::shared_ptr<spdlog::logger> Log::logger_;
    std::shared_ptr<spdlog::logger> Log::customLogger_;

    void Log::init() {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        logger_ = spdlog::stdout_color_mt("vk");
        logger_->set_level(spdlog::level::trace);

        auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();

//        console_sink->set_color_mode(spdlog::color_mode::automatic);
//        console_sink->set_color(spdlog::level::info, "\033[35m"); //
//        console_sink->set_color(spdlog::level::warn, "\033[36m"); //
//        customLogger_ = std::make_shared<spdlog::logger>("logger", console_sink);
//        spdlog::register_logger(customLogger_);
//        customLogger_->set_level(spdlog::level::trace);
    }

} // yic