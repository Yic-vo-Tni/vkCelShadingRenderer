//
// Created by lenovo on 11/25/2023.
//

#include "Log.h"

namespace yic {

    std::shared_ptr<spdlog::logger> Log::logger_;
    std::shared_ptr<spdlog::logger> Log::customLogger_;
    std::vector<spdlog::sink_ptr> Log::mSinks;

    void Log::init() {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();  // 标准控制台 sink
        mSinks.emplace_back(console_sink);
        logger_ = std::make_shared<spdlog::logger>("vk", mSinks.begin(), mSinks.end());
        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);
        spdlog::set_level(spdlog::level::trace);

    }

} // yic