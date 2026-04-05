#pragma once
#include <cstdint>
#include <string_view>
#include <vector>

namespace mantle {
    std::vector<uint32_t> load_spv(std::string_view path);
}

