// Copyright 2026 Mantle
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "mantle/core/types.h"
#include "mantle/renderer/types.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace mantle {

    struct TextureUpload {
        BufferHandle staging;
        ImageHandle  image;
        u32          mip_count;
        u32          mip_offsets[16];
    };

    MANTLE_HANDLE(MeshHandle);
    MANTLE_HANDLE(MaterialHandle);
    MANTLE_HANDLE(SceneHandle);

    struct SubMeshInfo {
        u32   vertex_count;
        u32   vertex_offset;
        u16   vertex_stride;
        u32   index_count;
        u32   index_offset;
        u32   attr_flags;
        float bounding_min[3];
        float bounding_max[3];
    };

    struct MeshData {
        BufferHandle                  vertex_buffer;
        BufferHandle                  index_buffer;
        u32                           vertex_count;
        u32                           index_count;
        u32                           lod_count;
        u32                           lod_first_submesh[4];
        std::pmr::vector<SubMeshInfo> submeshes;
    };

    struct MaterialData {
        float    base_color[4];
        float    metallic;
        float    roughness;
        float    emissive[3];
        float    alpha_cutoff;
        float    normal_scale;
        float    emissive_strength;
        uint32_t flags;
        // Bindless texture indices (UINT32_MAX = none)
        u32 bindless_basecolor = UINT32_MAX;
        u32 bindless_normal = UINT32_MAX;
        u32 bindless_mr = UINT32_MAX; // metallic-roughness
        u32 bindless_emissive = UINT32_MAX;
        u32 bindless_occlusion = UINT32_MAX;
        u32 bindless_sampler = UINT32_MAX;
    };

    struct MaterialGPU {
        glm::vec4 base_color;             // 16B
        glm::vec4 metallic_roughness;     // 16B (x=metallic, y=roughness, z=emissive_strength)
        glm::vec4 emissive_alpha_cutoff;  // 16B (rgb=emissive, w=alpha_cutoff)
        u32       base_color_tex;         // 4B
        u32       normal_tex;             // 4B
        u32       metallic_roughness_tex; // 4B
        u32       emissive_tex;           // 4B
        u32       occlusion_tex;          // 4B
        u32       sampler_idx;            // 4B
        u32       flags;                  // 4B
        u32       _padding[5];            // 20B align to 96B stride (shader uses idx * 96)
    };
    static_assert(sizeof(MaterialGPU) == 96, "MaterialGPU size mismatch");

} // namespace mantle
