// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "glm/gtc/noise.hpp"

#include <concepts>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "core/types.h"

namespace mantle {
    template <typename Fn>
    concept CNoise3D = requires(Fn fn, glm::vec3 v) {
        { fn(v) } -> std::convertible_to<f32>;
    };

    template <typename Fn>
    concept CNoise2D = requires(Fn fn, glm::vec2 v) {
        { fn(v) } -> std::convertible_to<f32>;
    };

    template <typename Fn>
    inline static constexpr bool is_valid_noise_v = CNoise2D<Fn> || CNoise3D<Fn>;

    inline f32 perlin2(glm::vec2 p) { return glm::perlin(p); }
    inline f32 perlin3(glm::vec3 p) { return glm::perlin(p); }
    inline f32 simplex2(glm::vec2 p) { return glm::simplex(p); }
    inline f32 simplex3(glm::vec3 p) { return glm::simplex(p); }
} // namespace mantle
