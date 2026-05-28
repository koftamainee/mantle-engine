#pragma once
#include "camera/camera.h"
#include "renderer/renderer.h"
#include "window/window.h"

#include "core/memory/arena_allocator.h"
#include "core/memory/virtual_heap.h"
#include "world/chunk_generation_system.h"

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
        Renderer m_renderer{};
        Camera m_camera{};
        ChunkGenerationSystem m_chunk_generation_system{};

        OSMemory m_os_memory;
        VirtualHeap m_heap;
        ArenaAllocator m_scratch_arena;

        ArenaAllocator m_rendering_arena;

        GraphicsPipelineHandle m_blit_pipeline{};
        ComputePipelineHandle m_dda_pipeline{};
        BufferHandle m_chunk_buffer{};

        SamplerHandle m_blit_sampler{};
        u32 m_blit_sampler_index{};

        f32 m_last_time = 0.0f;

        f32 m_mouse_x = 0.0f;
        f32 m_mouse_y = 0.0f;

        f32 m_camera_speed = m_base_camera_speed;

        static constexpr f32 m_base_camera_speed = 5.0f;
        static constexpr f32 m_mouse_sensitivity = 0.5f;
    };
} // namespace mantle
