// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/glm.hpp>

#include "mantle/core/assert.h"
#include "mantle/core/macros.h"
#include "mantle/core/memory/arena_allocator.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/types.h"
#include "mantle/ecs/ecs.h"
#include "mantle/renderer/frame_graph.h"
#include "mantle/renderer/gpu_resource_manager.h"
#include "mantle/renderer/renderer.h"
#include "mantle/renderer/types.h"

namespace mantle {

    struct SimpleMesh {
        BufferHandle vertex_buffer;
        BufferHandle index_buffer;
        u32          vertex_count;
        u32          index_count;
        u32          vertex_stride;
    };

    class SimpleRenderer final {
      public:
        SimpleRenderer() = default;
        ~SimpleRenderer() = default;

        void init(Renderer &renderer, MemoryBlock block);
        void destroy(Renderer &renderer);
        void add_passes(FrameGraph &graph, const Blackboard &blackboard, const Ecs &ecs);

      private:
        u32 add_mesh(Renderer &renderer, std::span<const u8> vertices, u32 vertex_stride,
                     std::span<const u32> indices);

        void create_floor_mesh(Renderer &renderer);
        void create_capsule_mesh(Renderer &renderer);

        bool             m_is_initialized = false;
        ArenaAllocator   m_arena{};
        spdlog::logger  *m_logger = nullptr;

        SimpleMesh m_meshes[2]{};
        u32        m_mesh_count = 0;

        GraphicsPipelineHandle m_pipeline{};
    };

} // namespace mantle
