// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <type_traits>

template <typename T>
concept EFlagsEnum = std::is_enum_v<T> && requires { T::MaxEnum; };

template <EFlagsEnum T>
inline T operator|(T a, T b) {
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) |
                          static_cast<std::underlying_type_t<T>>(b));
}

template <EFlagsEnum T>
inline bool operator&(T a, T b) {
    return static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b);
}
