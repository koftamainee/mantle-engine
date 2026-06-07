#pragma once
#include "renderer/renderer.h"
#include "window/window.h"

#include "core/concurrency/worker_pool.h"
#include "core/memory/arena_allocator.h"
#include "core/memory/virtual_heap.h"
#include "ecs/ecs.h"
#include "world/chunk_generation_system.h"
#include "world/chunk_meshing_system.h"
#include "world/chunk_rendering_system.h"
#include "world/chunk_storage_system.h"

namespace spdlog { class logger; }

namespace mantle {
    class Engine final {
      public:
        Engine() = default;
        ~Engine();

        MANTLE_NO_COPY_NO_MOVE(Engine);

        void init();
        void run();
        void destroy();

      private:
        void update(f32 delta_time);
        void render();

      private:
        bool m_is_initialized = false;
        Window m_window{};
        Ecs m_ecs{};
        Renderer m_renderer{};
        ChunkGenerationSystem m_chunk_generation_system{};

        OSMemory m_os_memory;
        VirtualHeap m_heap;
        ArenaAllocator m_scratch_arena;

        WorkerPool m_worker_pool;

        ArenaAllocator m_rendering_arena;
        ArenaAllocator m_meshing_arena;

        ChunkMeshingSystem m_chunk_meshing_system{};
        ChunkRenderingSystem m_chunk_rendering_system{};
        ChunkStorageSystem m_chunk_storage_system{};

        f32 m_last_time = 0.0f;
        f32 m_fps_timer = 0.0f;
        u32 m_fps_frame_count = 0;
        f32 m_fps_frametime_accum = 0.0f;
        f32 m_fps_frametime_min = 1e10f;
        f32 m_fps_frametime_max = 0.0f;

        f32 m_fps_begin_accum = 0.0f;
        f32 m_fps_exec_accum = 0.0f;
        f32 m_fps_end_accum = 0.0f;

        spdlog::logger *m_logger = nullptr;
    };
} // namespace mantle
