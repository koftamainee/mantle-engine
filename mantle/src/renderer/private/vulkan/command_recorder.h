#pragma once
#include <span>
#include <vulkan/vulkan.h>
#include "core/types.h"
#include "renderer/gpu_resource_manager.h"
#include "renderer/types.h"

namespace mantle {
    class CommandRecorder final {
      public:
        CommandRecorder() = default;
        ~CommandRecorder() = default;

        CommandRecorder(const CommandRecorder &) = delete;
        CommandRecorder(CommandRecorder &&) noexcept = delete;
        CommandRecorder &operator=(const CommandRecorder &) = delete;
        CommandRecorder &operator=(CommandRecorder &&) noexcept = delete;

        void set_command_buffer(VkCommandBuffer cmd);

        void image_barrier(const ImageBarrier &barrier);
        void pipeline_barriers(std::span<const ImageBarrier> barriers);

        void begin_rendering(const RenderingInfo &info);
        void end_rendering();

        void bind_graphics_pipeline(GraphicsPipelineHandle pipeline);
        void bind_compute_pipeline(ComputePipelineHandle pipeline);

        void set_viewport(f32 x, f32 y, f32 width, f32 height);
        void set_scissor(i32 x, i32 y, u32 width, u32 height);

        void draw(const DrawInfo &info);
        void draw_indexed(const DrawIndexedInfo &info);

        void dispatch(u32 x, u32 y, u32 z);

        void copy_buffer(BufferHandle src, BufferHandle dst, usize size,
                         usize src_offset = 0, usize dst_offset = 0);
        void copy_buffer_to_image(BufferHandle src, ImageHandle dst, u32 width,
                                  u32 height);

        void push_constants(const void *data, u32 size);
        void bind_descriptor_set();

      private:
        VkCommandBuffer m_cmd = VK_NULL_HANDLE;
        GPUResourceManager *m_resources = nullptr;
    };
} // namespace mantle
