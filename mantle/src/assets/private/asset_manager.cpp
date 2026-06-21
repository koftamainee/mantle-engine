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

#include "mantle/assets/asset_manager.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>

#include <flecs.h>

#include "impl.h"

namespace mantle {

    void AssetManager::init(Renderer &renderer, MemoryBlock block) {
        MANTLE_CHECK(!m_is_initialized);

        m_impl = new Impl();
        m_impl->renderer = &renderer;
        m_impl->logger = m_logger;

        m_impl->meshes.reserve(64);
        m_impl->materials.reserve(64);
        m_impl->scenes.reserve(8);

        m_logger = spdlog::get("assets").get();
        m_is_initialized = true;
        m_logger->info("AssetManager initialized");
    }

    void AssetManager::destroy() {
        if (m_is_initialized) {
            for (auto &scene : m_impl->scenes) {
                if (scene.loaded) {
                    mintload_MscbUnload(&scene.scb);
                    scene.loaded = false;
                }
            }
            for (auto &mat : m_impl->materials) {
                if (mat.loaded) {
                    mintload_MmatUnload(&mat.mint_mat);
                    mat.loaded = false;
                }
            }
            for (auto &mesh : m_impl->meshes) {
                if (mesh.loaded) {
                    if (mesh.data.vertex_buffer.is_valid()) {
                        m_impl->renderer->resource_manager().destroy_buffer(mesh.data.vertex_buffer,
                                                                            true);
                    }
                    if (mesh.data.index_buffer.is_valid()) {
                        m_impl->renderer->resource_manager().destroy_buffer(mesh.data.index_buffer,
                                                                            true);
                    }
                    mintload_MmeshUnload(&mesh.mint_mesh);
                    mesh.loaded = false;
                }
            }
            delete m_impl;
            m_impl = nullptr;
            m_is_initialized = false;
        }
    }

    SceneHandle AssetManager::preload(std::string_view scene_uuid) {
        MANTLE_CHECK(m_is_initialized);

        SceneHandle handle;
        handle.index = static_cast<u32>(m_impl->scenes.size());
        handle.generation = 1;

        auto &ls = m_impl->scenes.emplace_back();
        ls.base_path = m_impl->asset_base_path;

        std::string path = ls.base_path + "/scenes/" + std::string(scene_uuid) + ".mscb";

        MintloadResult r = mintload_MscbLoad(path.c_str(), &ls.scb);
        if (r != MINTLOAD_SUCCESS) {
            m_logger->error("Failed to load scene: {} (err={})", path, static_cast<int>(r));
            return handle;
        }

        u32 entity_count = mintload_MscbEntityCount(&ls.scb);
        ls.entity_mesh_handles.resize(entity_count);
        ls.entity_material_handles.resize(entity_count);

        // First pass: collect all unique mesh/material UUIDs and load them
        for (u32 ei = 0; ei < entity_count; ei++) {
            MintScbEntity entity = mintload_MscbEntity(&ls.scb, ei);
            for (u32 ci = 0; ci < entity.comp_count; ci++) {
                MintScbComponent comp = mintload_MscbEntityComp(&ls.scb, ei, ci);
                if (comp.type_hash == MINTLOAD_COMP_MESH_RENDERER) {
                    // Parse MeshRenderer component data:
                    // [0..15] mesh UUID
                    // [16..19] surface_offset (int32)
                    // [20..23] material_count (uint32)
                    // [24..] material UUIDs

                    const uint8_t *mesh_uuid = comp.data;
                    u32            mesh_idx = m_impl->find_or_load_mesh(mesh_uuid, ls.base_path);

                    MeshHandle mh;
                    mh.index = mesh_idx;
                    mh.generation = 0;
                    ls.entity_mesh_handles[ei] = mh;

                    u32 mat_count;
                    memcpy(&mat_count, comp.data + 20, sizeof(mat_count));

                    auto &mat_handles = ls.entity_material_handles[ei];
                    mat_handles.reserve(mat_count);
                    for (u32 mi = 0; mi < mat_count; mi++) {
                        const uint8_t *mat_uuid = comp.data + 24 + mi * 16;
                        u32 mat_idx = m_impl->find_or_load_material(mat_uuid, ls.base_path);

                        MaterialHandle mat_h;
                        mat_h.index = mat_idx;
                        mat_h.generation = 1;
                        mat_handles.push_back(mat_h);
                    }
                }
            }
        }

        ls.loaded = true;
        m_logger->info("Scene loaded: {} ({} entities, {} meshes)", path, entity_count,
                       m_impl->meshes.size());
        return handle;
    }

    void AssetManager::unload(SceneHandle scene) {
        MANTLE_CHECK(m_is_initialized);
        if (scene.index < m_impl->scenes.size() && m_impl->scenes[scene.index].loaded) {
            auto &ls = m_impl->scenes[scene.index];
            mintload_MscbUnload(&ls.scb);
            ls.loaded = false;
        }
    }

    flecs::entity AssetManager::instantiate(flecs::world &world, SceneHandle scene,
                                            LocalTransform transform) {
        MANTLE_CHECK(m_is_initialized);

        if (scene.index >= m_impl->scenes.size() || !m_impl->scenes[scene.index].loaded) {
            m_logger->error("AssetManager::instantiate: invalid or unloaded scene handle");
            return flecs::entity(world);
        }

        auto &ls = m_impl->scenes[scene.index];
        u32   entity_count = mintload_MscbEntityCount(&ls.scb);

        std::pmr::vector<flecs::entity> entities(ls.entity_mesh_handles.get_allocator().resource());
        entities.reserve(entity_count);

        // First pass: create all entities
        for (u32 ei = 0; ei < entity_count; ei++) {
            MintScbEntity mentity = mintload_MscbEntity(&ls.scb, ei);
            auto          e = world.entity();
            if (mentity.name && mentity.name_len > 0) {
                e.set_name(std::string_view(mentity.name, mentity.name_len).data());
            }
            entities.push_back(e);

            // Apply components
            LocalTransform lt = transform;
            for (u32 ci = 0; ci < mentity.comp_count; ci++) {
                MintScbComponent comp = mintload_MscbEntityComp(&ls.scb, ei, ci);
                if (comp.type_hash == MINTLOAD_COMP_TRANSFORM) {
                    // Transform layout: float translation[3] (12B) + float rotation[4] (16B) +
                    // float scale[3] (12B) = 40B
                    memcpy(&lt.translation, comp.data, sizeof(float) * 3);
                    memcpy(&lt.rotation, comp.data + 12, sizeof(float) * 4);
                    memcpy(&lt.scale, comp.data + 28, sizeof(float) * 3);
                } else if (comp.type_hash == MINTLOAD_COMP_MESH_RENDERER) {
                    MeshRenderer mr {};
                    mr.mesh = ls.entity_mesh_handles[ei];
                    auto &mat_handles = ls.entity_material_handles[ei];
                    for (usize mi = 0; mi < mat_handles.size() && mi < 10; mi++) {
                        mr.materials[mi] = mat_handles[mi];
                    }
                    e.set<MeshRenderer>(mr);
                }
            }

            e.set<LocalTransform>(lt);
        }

        // Second pass: set up parent-child hierarchy via C API
        ecs_world_t *ecs = world.c_ptr();
        for (u32 ei = 0; ei < entity_count; ei++) {
            MintScbEntity mentity = mintload_MscbEntity(&ls.scb, ei);
            if (mentity.parent_index >= 0 &&
                static_cast<u32>(mentity.parent_index) < entity_count &&
                mentity.parent_index != static_cast<i32>(ei)) {
                ecs_add_pair(ecs, entities[ei].id(), EcsChildOf,
                             entities[mentity.parent_index].id());
            }
        }

        return entities.empty() ? flecs::entity(world) : entities[0];
    }

    u32 AssetManager::mesh_count() const {
        MANTLE_CHECK(m_is_initialized);
        return static_cast<u32>(m_impl->meshes.size());
    }

    const MeshData &AssetManager::mesh_data_by_index(u32 index) const {
        MANTLE_CHECK(m_is_initialized && index < m_impl->meshes.size());
        return m_impl->meshes[index].data;
    }

    const MeshData *AssetManager::mesh_data(MeshHandle handle) const {
        MANTLE_CHECK(m_is_initialized);
        if (!handle.is_valid() || handle.index >= m_impl->meshes.size()) {
            return nullptr;
        }
        if (!m_impl->meshes[handle.index].loaded) {
            return nullptr;
        }
        return &m_impl->meshes[handle.index].data;
    }

} // namespace mantle
