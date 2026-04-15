#pragma once
#include "core/memory/arena_allocator.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"
#include "gpu_resource_manager.h"
#include "render_graph.h"
#include "renderer/types.h"

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

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) noexcept = delete;
        Renderer &operator=(Renderer &&) noexcept = delete;

        void init(const Window &window, bool vsync, VirtualHeap *heap,
                  ArenaAllocator *scratch_arena);
        void destroy();

        Result begin_frame();
        Result end_frame();

        GPUResourceManager &resource_manager();
        ImageHandle backbuffer() const;

        void execute(const CompiledRenderGraph &render_graph);

        void resize_swapchain(u32 width, u32 height);
        SwapchainInfo get_swapchain_info();

      private:
        bool m_is_initialized = false;

        struct Impl;
        Impl *m_impl = nullptr;
    };

} // namespace mantle
