#pragma once

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

        void init(VulkanBackend *backend, u8 frames_in_flight);
        void destroy();

        FrameResult begin_frame(FrameContext &out_ctx);
        FrameResult end_frame(const FrameContext &ctx);

        void on_swapchain_rebuilt(u32 new_image_count);

      private:
        // TODO
    };

} // namespace mantle
