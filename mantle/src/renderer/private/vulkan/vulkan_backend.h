#pragma once
#include "core/memory/arena_allocator.h"
#include "core/types.h"
#include "renderer/types.h"
#include "vulkan_context.h"
#include "vulkan_cpu_allocator.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "window/window.h"

namespace mantle {
    enum class SwapchainResult {
        Ok,
        Suboptimal,
        OutOfDate,
    };

    struct AcquiredImage final {
        SwapchainResult result;
        u32 image_index;
    };


    class VulkanBackend final {
      public:
        VulkanBackend() = default;
        ~VulkanBackend();

        MANTLE_NO_COPY_NO_MOVE(VulkanBackend);

        void init(const Window &window, bool vsync, VirtualHeap *heap,
                  ArenaAllocator *scratch_arena);
        void destroy();
        void wait_idle() const;

        void rebuild_swapchain(u32 width, u32 height);
        SwapchainInfo get_swapchain_info() const;


      private:
        friend class FrameScheduler;
        friend class GPUResourceManager;

        AcquiredImage acquire_next_image(VkSemaphore image_available) const;
        SwapchainResult present(u32 image_index,
                                VkSemaphore render_finished) const;

        bool m_is_initialized = false;

        VulkanContext m_context{};
        VulkanDevice m_device{};
        VulkanSwapchain m_swapchain{};

        VirtualHeap *m_heap = nullptr;
        ArenaAllocator *m_scratch_arena = nullptr;
        TlsfAllocator m_tlsf_allocator{};
        VulkanCPUAllocator m_vk_allocator{};

        bool m_vsync = true;
    };


} // namespace mantle
