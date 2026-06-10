// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "core/concurrency/worker_pool.h"
#include "core/memory/arena_allocator.h"
#include "core/memory/tlsf_allocator.h"
#include "core/memory/virtual_heap.h"
#include "ecs/ecs.h"
#include "physics/character_controller.h"
#include "physics/physics_system.h"
#include "renderer/renderer.h"
#include "window/window.h"
#include "world/chunk_generation_system.h"
#include "world/chunk_meshing_system.h"
#include "world/chunk_rendering_system.h"
#include "world/chunk_storage_system.h"

namespace spdlog {
    class logger;
}

namespace mantle {
    class Engine final {
      public:
        MANTLE_DEFAULT_INIT(Engine);

        void init();
        void run();
        void destroy();

      private:
        void update(f32 delta_time);
        void render();

      private:
        // Memory
        OSMemory    m_os_memory;
        VirtualHeap m_heap;

        // Window
        Window m_window {};

        // Parallelism
        WorkerPool m_worker_pool;

        // Core Systems
        PhysicsSystem m_physics_system {};
        CharacterController m_character {};
        Ecs           m_ecs {};

        // Graphics
        Renderer m_renderer {};

        // Old world (to be removed)
        ChunkGenerationSystem m_chunk_generation_system {};
        ChunkStorageSystem    m_chunk_storage_system {};
        ChunkRenderingSystem  m_chunk_rendering_system {};
        ChunkMeshingSystem    m_chunk_meshing_system {};

        u64 m_last_time = 0;
        f32 m_fps_timer = 0.0f;
        u32 m_fps_frame_count = 0;
        f32 m_fps_frametime_accum = 0.0f;
        f32 m_fps_frametime_min = 1e10f;
        f32 m_fps_frametime_max = 0.0f;

        f32 m_fps_begin_accum = 0.0f;
        f32 m_fps_exec_accum = 0.0f;
        f32 m_fps_end_accum = 0.0f;

        bool            m_is_initialized = false;
        spdlog::logger *m_logger = nullptr;
    };
} // namespace mantle
