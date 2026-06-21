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

#include <string_view>

#include "mantle/assets/types.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/ecs/components.h"

namespace spdlog {
    class logger;
}

namespace flecs {
    class world;
    class entity;
} // namespace flecs

namespace mantle {
    class Renderer;

    class AssetManager final {
        friend class Engine;
        friend class Renderer;

      public:
        AssetManager() = default;
        MANTLE_NO_COPY_NO_MOVE(AssetManager);

        SceneHandle preload(std::string_view uuid);

        void unload(SceneHandle scene);

        flecs::entity instantiate(flecs::world &world, SceneHandle scene,
                                  LocalTransform transform = {});

        // Access for render pipeline
        u32             mesh_count() const;
        const MeshData &mesh_data_by_index(u32 index) const;
        const MeshData *mesh_data(MeshHandle handle) const;

      private:
        void init(Renderer &renderer, MemoryBlock mem);
        void destroy();

        bool m_is_initialized = false;
        struct Impl;
        Impl           *m_impl = nullptr;
        spdlog::logger *m_logger = nullptr;
    };


} // namespace mantle
