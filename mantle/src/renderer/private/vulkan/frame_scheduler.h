// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "command_recorder.h"
#include "mantle/core/memory/arena_allocator.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/memory/pmr/arena_resource.h"
#include "mantle/core/memory/tlsf_allocator.h"
#include "mantle/core/types.h"

namespace spdlog {
    class logger;
}

namespace mantle {

    class VulkanBackend;
    class CommandRecorder;

    enum class FrameResult {
        Ok,
        NeedsResize,
    };

    struct FrameContext final {
        CommandRecorder *cmd;
        u32              image_index;
        u32              frame_index;
    };

    class FrameScheduler final {
      public:
        MANTLE_DEFAULT_INIT(FrameScheduler);

        void init(VulkanBackend *backend, GPUResourceManager *resource_manager,
                  u32 frames_in_flight, ArenaAllocator &frame_arena, TlsfAllocator *perm_allocator);
        void destroy();

        FrameResult begin_frame(FrameContext &out_ctx);
        FrameResult end_frame(const FrameContext &ctx);

        void on_swapchain_rebuilt(u32 new_image_count) const;

        ArenaAllocator &frame_arena() { return *m_frame_arena; }
        ArenaResource  &frame_arena_resource() { return m_pmr; }

      private:
        struct FrameData {
            VkFence         fence;
            VkSemaphore     image_available;
            VkCommandBuffer cmd;
        };

        VkSemaphore *m_render_finished = nullptr;

        bool           m_is_initialized = false;
        VulkanBackend *m_backend = nullptr;

        ArenaAllocator *m_frame_arena = nullptr;
        ArenaResource   m_pmr {};

        u32 m_frames_in_flight = 0;
        u32 m_swapchain_image_count = 0;
        u32 m_current_frame = 0;

        VkCommandPool   m_command_pool {};
        FrameData      *m_frames = nullptr;
        CommandRecorder m_recorder;

        spdlog::logger *m_logger = nullptr;
    };

} // namespace mantle
