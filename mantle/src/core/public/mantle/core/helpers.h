// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <span>

namespace mantle {
    template <typename T>
    constexpr std::span<T> span(T &value) {
        return std::span<T>(&value, 1);
    }
} // namespace mantle
