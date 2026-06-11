// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/renderer/utils.h"

#include <fstream>

#include "mantle/core/assert.h"

namespace mantle {
    void load_spv(std::string_view path, std::pmr::vector<u32> &out) {
        std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
        MANTLE_FATAL(!file.is_open(), "failed to open shader file");

        long size = file.tellg();
        MANTLE_FATAL(size % 4 != 0, "Invalid SPIR-V shader");

        file.seekg(0);
        out.resize(size / 4);
        file.read(reinterpret_cast<char *>(out.data()), size);
    }
} // namespace mantle
