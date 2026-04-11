#pragma once
#include "core/memory/arena_allocator.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"
#include "glm/glm.hpp"

namespace mantle {

    class Window;

    class Renderer final {
      public:
        enum class Result {
            Ok,
            FrameNeedsResize,
            InvalidMeshHandle,
        };

        Renderer();
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) noexcept = delete;
        Renderer &operator=(Renderer &&) noexcept = delete;

        void init(const Window &window, VirtualHeap *heap, ArenaAllocator *scratch_arena);
        void destroy();

        void set_camera(const glm::mat4 &view,
                        const glm::mat4 &projection) const;

        Result begin_frame() const;
        Result end_frame() const;

        void begin_pass() const;
        void end_pass() const;

        void resize(u32 width, u32 height) const;


      private:
        bool m_is_initialized = false;

        struct Impl;
        Impl *m_impl = nullptr;
    };

} // namespace mantle
