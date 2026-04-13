#pragma once
#include "core/memory/arena_allocator.h"
#include "core/types.h"
#include "renderer/handles.h"
#include "window/window.h"

namespace mantle {

    enum class AcquireResult {
        Ok,
        Suboptimal,
        OutOfDate,
    };

    struct AcquiredImage final {
        AcquireResult result;
        u32 image_index;
    };

    struct SwapchainInfo final {
        u32 image_count;
        u32 width;
        u32 height;
        ImageFormat surface_format;
        ImageFormat depth_format;
    };

    class VulkanBackend final {
      public:
        VulkanBackend() = default;
        ~VulkanBackend();

        VulkanBackend(const VulkanBackend &) = delete;
        VulkanBackend &operator=(const VulkanBackend &) = delete;
        VulkanBackend(VulkanBackend &&) noexcept = delete;
        VulkanBackend &operator=(VulkanBackend &&) noexcept = delete;

        void init(const Window &window, bool vsync, VirtualHeap *heap,
                  ArenaAllocator *scratch_arena);
        void destroy();
        void wait_idle() const;

        void rebuild_swapchain(u32 width, u32 height);
        SwapchainInfo get_swapchain_info() const;

        AcquiredImage acquire_next_image(u32 semaphore_index) const;
        void present(u32 image_index, u32 semaphore_index) const;

        struct InternalAccess;
        InternalAccess get_internal() const;

      private:
        struct Impl;
        Impl *m_impl = nullptr;
    };


} // namespace mantle
