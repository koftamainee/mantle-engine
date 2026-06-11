// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <string_view>
#include <vector>

#include "mantle/core/types.h"

namespace mantle {
    void load_spv(std::string_view path, std::pmr::vector<u32> &out);
} // namespace mantle
