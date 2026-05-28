#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "core/types.h"
#include "noise.h"

namespace mantle {
    namespace detail {
        glm::vec2 seed_offset(u32 seed);
        glm::vec3 seed_offset_3d(u32 seed);
    }

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    struct Sampler {
        Fn noise_fn;
        u32 seed = 0;
        f32 scale = 1.0f;
        i32 octaves = 1;
        f32 lacunarity = 2.0f;
        f32 gain = 0.5f;

        f32 sample(glm::vec2 p) const
            requires CNoise2D<Fn>;
        f32 sample(glm::vec3 p) const
            requires CNoise3D<Fn>;

        template <typename Fn2>
            requires CNoise2D<Fn2>
        f32 sample_warped(glm::vec2 p, const Sampler<Fn2>& noise2,
                          f32 warp_scale = 1.0f) const
            requires CNoise2D<Fn>;

        template <typename Fn2>
            requires CNoise3D<Fn2>
        f32 sample_warped(glm::vec3 p, const Sampler<Fn2>& noise2,
                          f32 warp_scale = 1.0f) const
            requires CNoise3D<Fn>;

        f32 sample_range(glm::vec2 p, f32 min, f32 max) const
            requires CNoise2D<Fn>;
        f32 sample_range(glm::vec3 p, f32 min, f32 max) const
            requires CNoise3D<Fn>;

        template <typename Fn2>
            requires CNoise2D<Fn2>
        f32 sample_range_warped(glm::vec2 p, f32 min, f32 max,
                                const Sampler<Fn2>& noise2,
                                f32 warp_scale = 1.0f) const
            requires CNoise2D<Fn>;

        template <typename Fn2>
            requires CNoise3D<Fn2>
        f32 sample_range_warped(glm::vec3 p, f32 min, f32 max,
                                const Sampler<Fn2>& noise2,
                                f32 warp_scale = 1.0f) const
            requires CNoise3D<Fn>;
    };

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    f32 Sampler<Fn>::sample(glm::vec2 p) const
        requires CNoise2D<Fn>
    {
        p = p * scale + detail::seed_offset(seed);

        f32 total = 0.0f;
        f32 amp = 1.0f;
        f32 max_amp = 0.0f;

        for (i32 i = 0; i < octaves; i++) {
            total += amp * noise_fn(p);
            max_amp += amp;
            amp *= gain;
            p *= lacunarity;
        }

        return max_amp > 0.0f ? total / max_amp : 0.0f;
    }

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    f32 Sampler<Fn>::sample(glm::vec3 p) const
        requires CNoise3D<Fn>
    {
        p = p * scale + detail::seed_offset_3d(seed);

        f32 total = 0.0f;
        f32 amp = 1.0f;
        f32 max_amp = 0.0f;

        for (i32 i = 0; i < octaves; i++) {
            total += amp * noise_fn(p);
            max_amp += amp;
            amp *= gain;
            p *= lacunarity;
        }

        return max_amp > 0.0f ? total / max_amp : 0.0f;
    }

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    template <typename Fn2>
        requires CNoise2D<Fn2>
    f32 Sampler<Fn>::sample_warped(glm::vec2 p, const Sampler<Fn2>& noise2,
                                   f32 warp_scale) const
        requires CNoise2D<Fn>
    {
        glm::vec2 warp{
            noise2.sample(p + glm::vec2{5.2f, 1.3f}),
            noise2.sample(p + glm::vec2{9.7f, 3.5f}),
        };
        return sample(p + warp * warp_scale);
    }

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    template <typename Fn2>
        requires CNoise3D<Fn2>
    f32 Sampler<Fn>::sample_warped(glm::vec3 p, const Sampler<Fn2>& noise2,
                                   f32 warp_scale) const
        requires CNoise3D<Fn>
    {
        glm::vec3 warp{
            noise2.sample(p + glm::vec3{5.2f, 1.3f, 7.8f}),
            noise2.sample(p + glm::vec3{9.7f, 3.5f, 2.1f}),
            noise2.sample(p + glm::vec3{6.4f, 8.9f, 4.6f}),
        };
        return sample(p + warp * warp_scale);
    }

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    f32 Sampler<Fn>::sample_range(glm::vec2 p, f32 min, f32 max) const
        requires CNoise2D<Fn>
    {
        f32 t = sample(p);
        t = t * 0.5f + 0.5f;
        return min + t * (max - min);
    }

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    f32 Sampler<Fn>::sample_range(glm::vec3 p, f32 min, f32 max) const
        requires CNoise3D<Fn>
    {
        f32 t = sample(p);
        t = t * 0.5f + 0.5f;
        return min + t * (max - min);
    }

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    template <typename Fn2>
        requires CNoise2D<Fn2>
    f32 Sampler<Fn>::sample_range_warped(glm::vec2 p, f32 min, f32 max,
                                         const Sampler<Fn2>& noise2,
                                         f32 warp_scale) const
        requires CNoise2D<Fn>
    {
        f32 t = sample_warped(p, noise2, warp_scale);
        t = t * 0.5f + 0.5f;
        return min + t * (max - min);
    }

    template <typename Fn>
        requires is_valid_noise_v<Fn>
    template <typename Fn2>
        requires CNoise3D<Fn2>
    f32 Sampler<Fn>::sample_range_warped(glm::vec3 p, f32 min, f32 max,
                                         const Sampler<Fn2>& noise2,
                                         f32 warp_scale) const
        requires CNoise3D<Fn>
    {
        f32 t = sample_warped(p, noise2, warp_scale);
        t = t * 0.5f + 0.5f;
        return min + t * (max - min);
    }

} // namespace mantle
