#pragma once
#include "core/memory/arena_allocator.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"
#include "frame_graph.h"
#include "gpu_resource_manager.h"
#include "renderer/types.h"

namespace spdlog { class logger; }

namespace mantle {

    class Window;

    class Renderer final {
      public:
        enum class Result {
            Ok,
            FrameNeedsResize,
        };

        Renderer() = default;
        ~Renderer();

        MANTLE_NO_COPY_NO_MOVE(Renderer);

        void init(const Window &window, bool vsync, VirtualHeap *heap,
                  ArenaAllocator *scratch_arena);
        void destroy();

        Result begin_frame();
        Result end_frame();

        GPUResourceManager &resource_manager();
        ImageHandle backbuffer() const;

        void execute(FrameGraph &render_graph);

        void resize_swapchain(u32 width, u32 height);
        SwapchainInfo get_swapchain_info();
        std::string_view gpu_name() const;
        u64 vram_bytes() const;

      private:
        bool m_is_initialized = false;

        struct Impl;
        Impl *m_impl = nullptr;

        spdlog::logger *m_logger = nullptr;
    };

} // namespace mantle
