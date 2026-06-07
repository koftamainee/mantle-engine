#pragma once
#include <cstdint>
#include <string_view>
#include <spdlog/logger.h>

#include "core/types.h"

namespace mantle {
    void log_system_info(spdlog::logger *logger, std::string_view gpu_name = {},
                         uint64_t vram_bytes = 0, u32 window_w = 0,
                         u32 window_h = 0, bool vsync = false,
                         f32 refresh_rate = 0.0f, bool fullscreen = false);
}
