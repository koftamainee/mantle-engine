#include "frame_scheduler.h"

namespace mantle {

    FrameScheduler::~FrameScheduler() {}

    void FrameScheduler::init(VulkanBackend *backend, u8 frames_in_flight) {}

    void FrameScheduler::destroy() {}

    FrameResult FrameScheduler::begin_frame(FrameContext &out_ctx) {}

    FrameResult FrameScheduler::end_frame(const FrameContext &ctx) {}

    void FrameScheduler::on_swapchain_rebuilt(u32 new_image_count) {}
} // namespace mantle
