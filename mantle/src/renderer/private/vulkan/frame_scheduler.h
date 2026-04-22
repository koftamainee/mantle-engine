#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>


#include "command_recorder.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"

namespace mantle {

    class VulkanBackend;
    class CommandRecorder;

    enum class FrameResult {
        Ok,
        NeedsResize,
    };

    struct FrameContext final {
        CommandRecorder *cmd;
        u32 image_index;
        u32 frame_index;
    };

    class FrameScheduler final {
      public:
        FrameScheduler() = default;
        ~FrameScheduler();

        MANTLE_NO_COPY_NO_MOVE(FrameScheduler);

        void init(VulkanBackend *backend, GPUResourceManager *resource_manager,
                  u32 frames_in_flight, VirtualHeap *heap);
        void destroy();

        FrameResult begin_frame(FrameContext &out_ctx);
        FrameResult end_frame(const FrameContext &ctx);

        void on_swapchain_rebuilt(u32 new_image_count) const;

      private:
        struct FrameData {
            VkFence fence;
            VkSemaphore image_available;
            VkCommandBuffer cmd;
        };

        VkSemaphore *m_render_finished = nullptr;

        bool m_is_initialized = false;
        VulkanBackend *m_backend = nullptr;

        ArenaAllocator m_frame_arena{};
        ArenaResource m_pmr{};

        u32 m_frames_in_flight = 0;
        u32 m_swapchain_image_count = 0;
        u32 m_current_frame = 0;

        VkCommandPool m_command_pool{};
        FrameData *m_frames = nullptr;
        CommandRecorder m_recorder;
    };

} // namespace mantle
