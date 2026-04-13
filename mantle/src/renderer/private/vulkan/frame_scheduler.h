#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>


#include "command_recorder.h"
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

        FrameScheduler(const FrameScheduler &) = delete;
        FrameScheduler &operator=(const FrameScheduler &) = delete;
        FrameScheduler(FrameScheduler &&) noexcept = delete;
        FrameScheduler &operator=(FrameScheduler &&) noexcept = delete;

        void init(VulkanBackend *backend, GPUResourceManager *resource_manager, u32 frames_in_flight);
        void destroy();

        FrameResult begin_frame(FrameContext &out_ctx);
        FrameResult end_frame(const FrameContext &ctx);

        void on_swapchain_rebuilt(u32 new_image_count) const;

      private:
        struct FrameData {
            VkFence fence;
            VkSemaphore image_available;
            VkSemaphore render_finished;
            VkCommandBuffer cmd;
        };

        bool m_is_initialized = false;
        VulkanBackend *m_backend = nullptr;

        u32 m_frames_in_flight = 0;
        u32 m_current_frame = 0;

        VkCommandPool m_command_pool{};
        FrameData *m_frames = nullptr;
        CommandRecorder m_recorder;
    };

} // namespace mantle
