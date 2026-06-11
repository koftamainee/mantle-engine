// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "mantle/engine/script.h"
#include "mantle/core/concurrency/worker_pool.h"
#include "mantle/core/memory/arena_allocator.h"
#include "mantle/core/memory/pmr/tlsf_resource.h"
#include "mantle/core/memory/tlsf_allocator.h"
#include "mantle/core/memory/virtual_heap.h"
#include "mantle/ecs/ecs.h"
#include "mantle/physics/character_controller.h"
#include "mantle/physics/physics_system.h"
#include "mantle/renderer/renderer.h"
#include "mantle/window/window.h"

#include <vector>

namespace spdlog {
    class logger;
}

namespace mantle {
    class SimpleRenderer;

    struct EngineConfig {
        Window::Properties window;
    };

    class Engine final {
      public:
        MANTLE_DEFAULT_INIT(Engine);

        static Engine *instance();

        void init(const EngineConfig &cfg);
        void run();
        void destroy();

        Ecs              &ecs();
        const Ecs        &ecs() const;
        Window           &window();
        const Window     &window() const;
        CharacterController &character();
        const CharacterController &character() const;

        void register_script(const ScriptCallbacks *callbacks);

      private:
        void process_awake_phase(f32 dt);
        void process_script_phase(ScriptPhase phase, f32 dt);
        void update(f32 dt);
        void render();

        static Engine *s_instance;

        OSMemory      m_os_memory;
        VirtualHeap   m_heap;
        TlsfAllocator m_persistent_allocator{};
        TlsfResource  m_heap_resource{};

        Window m_window {};

        WorkerPool m_worker_pool;

        PhysicsSystem       m_physics_system {};
        CharacterController m_character {};
        Ecs                 m_ecs {};
        
        Renderer m_renderer {};

        // Simple mesh renderer
        SimpleRenderer *m_simple_renderer = nullptr;

        // Scripts
        std::pmr::vector<const ScriptCallbacks *> m_scripts{&m_heap_resource};

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
