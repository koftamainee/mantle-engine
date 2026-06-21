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

#include <mintload/mmat.h>
#include <mintload/mmesh.h>
#include <mintload/mscb.h>
#include <spdlog/spdlog.h>

#include <cstdio>
#include <memory_resource>
#include <string>
#include <vector>

#include "mantle/assets/asset_manager.h"
#include "mantle/assets/types.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/memory/pmr/arena_resource.h"
#include "mantle/core/types.h"
#include "mantle/renderer/gpu_resource_manager.h"
#include "mantle/renderer/renderer.h"

namespace mantle {

    struct LoadedMesh {
        MintMesh mint_mesh {};
        MeshData data {};

        bool loaded = false;
    };

    struct LoadedMaterial {
        MintMaterial mint_mat {};
        MaterialData data {};

        bool loaded = false;
    };

    struct LoadedScene {
        MintScb                                            scb {};
        std::pmr::vector<MeshHandle>                       entity_mesh_handles {};
        std::pmr::vector<std::pmr::vector<MaterialHandle>> entity_material_handles {};
        std::string                                        base_path {};

        bool loaded = false;
    };

    struct AssetManager::Impl {
        Renderer                                  *renderer = nullptr;
        spdlog::logger                            *logger = nullptr;
        std::pmr::polymorphic_allocator<std::byte> alloc {};

        std::pmr::vector<LoadedMesh>     meshes {};
        std::pmr::vector<LoadedMaterial> materials {};
        std::pmr::vector<LoadedScene>    scenes {};

        std::string asset_base_path {"assets"};

        // UUID formatting helpers
        static void uuid_to_str(const uint8_t uuid[16], char *out) {
            static const char hex[17] = "0123456789abcdef";
            for (int i = 0; i < 16; i++) {
                *out++ = hex[uuid[i] >> 4];
                *out++ = hex[uuid[i] & 0xf];
                if (i == 3 || i == 5 || i == 7 || i == 9) {
                    *out++ = '-';
                }
            }
            *out = '\0';
        }

        // Find or load a mesh by UUID, return its index
        u32 find_or_load_mesh(const uint8_t uuid[16], const std::string &base) {
            char uuid_str[37];
            uuid_to_str(uuid, uuid_str);

            // Check if already loaded
            for (u32 i = 0; i < meshes.size(); i++) {
                if (!meshes[i].loaded) {
                    continue;
                }
                if (meshes[i].data.vertex_buffer.is_valid()) {
                    // We could compare UUIDs, but for now just assume unique
                }
            }

            // Determine mesh index
            u32 idx = static_cast<u32>(meshes.size());
            meshes.push_back({});
            auto &lm = meshes[idx];

            // Load .mmesh
            std::string    path = base + "/meshes/" + uuid_str + ".mmesh";
            MintloadResult r = mintload_MmeshLoad(path.c_str(), &lm.mint_mesh);
            if (r != MINTLOAD_SUCCESS) {
                logger->error("Failed to load mesh: {} (err={})", path, static_cast<int>(r));
                return idx;
            }

            // Create GPU vertex buffer
            usize      vertex_bytes = lm.mint_mesh.vertex_count * 48; // fixed stride
            BufferDesc vb_desc = {
                .size = vertex_bytes,
                .usage = BufferUsage::Vertex,
                .memory = MemoryType::CpuToGpu,
            };
            lm.data.vertex_buffer = renderer->resource_manager().create_buffer(vb_desc, true);
            renderer->resource_manager().update_buffer(lm.data.vertex_buffer,
                                                       lm.mint_mesh.vertex_data, vertex_bytes);

            // Create GPU index buffer
            usize      index_bytes = lm.mint_mesh.index_count * sizeof(u32);
            BufferDesc ib_desc = {
                .size = index_bytes,
                .usage = BufferUsage::Index,
                .memory = MemoryType::CpuToGpu,
            };
            lm.data.index_buffer = renderer->resource_manager().create_buffer(ib_desc, true);
            renderer->resource_manager().update_buffer(lm.data.index_buffer,
                                                       lm.mint_mesh.index_data, index_bytes);

            lm.data.vertex_count = lm.mint_mesh.vertex_count;
            lm.data.index_count = lm.mint_mesh.index_count;

            // Copy submesh info
            u32 sm_count = mintload_MmeshSubMeshCount(&lm.mint_mesh);
            lm.data.submeshes.reserve(sm_count);
            for (u32 sm_i = 0; sm_i < sm_count; sm_i++) {
                MintSubMesh msm = mintload_MmeshSubMesh(&lm.mint_mesh, sm_i);
                SubMeshInfo info = {
                    .vertex_count = msm.vertex_count,
                    .vertex_offset = msm.vertex_offset,
                    .vertex_stride = msm.vertex_stride,
                    .index_count = msm.index_count,
                    .index_offset = msm.index_offset,
                    .attr_flags = msm.flags,
                };
                for (int b = 0; b < 3; b++) {
                    info.bounding_min[b] = msm.bounding_min[b];
                    info.bounding_max[b] = msm.bounding_max[b];
                }
                lm.data.submeshes.push_back(info);
            }

            lm.loaded = true;
            return idx;
        }

        // Find or load a material by UUID, return its index
        u32 find_or_load_material(const uint8_t uuid[16], const std::string &base) {
            char uuid_str[37];
            uuid_to_str(uuid, uuid_str);

            u32 idx = static_cast<u32>(materials.size());
            materials.push_back({});
            auto &lm = materials[idx];

            std::string    path = base + "/materials/" + uuid_str + ".mmat";
            MintloadResult r = mintload_MmatLoad(path.c_str(), &lm.mint_mat);
            if (r != MINTLOAD_SUCCESS) {
                logger->error("Failed to load material: {} (err={})", path, static_cast<int>(r));
                return idx;
            }

            lm.data = {
                .base_color = {lm.mint_mat.base_color[0], lm.mint_mat.base_color[1],
                               lm.mint_mat.base_color[2], lm.mint_mat.base_color[3]},
                .metallic = lm.mint_mat.metallic,
                .roughness = lm.mint_mat.roughness,
                .emissive = {lm.mint_mat.emissive[0], lm.mint_mat.emissive[1],
                             lm.mint_mat.emissive[2]},
                .alpha_cutoff = lm.mint_mat.alpha_cutoff,
                .normal_scale = lm.mint_mat.normal_scale,
                .emissive_strength = lm.mint_mat.emissive_strength,
                .flags = lm.mint_mat.flags,
            };

            lm.loaded = true;
            return idx;
        }
    };

} // namespace mantle
