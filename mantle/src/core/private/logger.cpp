#include "core/logger.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace mantle {
    void init_logger() {
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink =
            std::make_shared<spdlog::sinks::basic_file_sink_mt>("mantle.log", true);

        auto meta_pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v";
        stdout_sink->set_pattern(meta_pattern);
        file_sink->set_pattern(meta_pattern);

        std::vector<spdlog::sink_ptr> sinks = {stdout_sink, file_sink};

        auto make_logger = [&](const char *name) {
            auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
            spdlog::register_logger(logger);
            return logger;
        };

        make_logger("engine");
        make_logger("renderer");
        make_logger("vulkan");
        make_logger("world");
        make_logger("window");
        make_logger("core");

        spdlog::set_default_logger(spdlog::get("engine"));

        auto raw_stdout = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        raw_stdout->set_pattern("%v");
        std::vector<spdlog::sink_ptr> raw_sinks = {raw_stdout, file_sink};
        auto raw = std::make_shared<spdlog::logger>("raw", raw_sinks.begin(), raw_sinks.end());
        spdlog::register_logger(raw);

#ifndef NDEBUG
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);
#else
        spdlog::set_level(spdlog::level::info);
        spdlog::flush_on(spdlog::level::info);
#endif
    }

    spdlog::logger *raw_logger() {
        return spdlog::get("raw").get();
    }
} // namespace mantle
