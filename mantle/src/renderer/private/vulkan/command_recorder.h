#pragma once
#include <span>
#include <vulkan/vulkan.h>
#include "core/memory/arena_allocator.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/types.h"
#include "renderer/gpu_resource_manager.h"
#include "renderer/types.h"
#include "types.h"

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
        void set_resource_manager(GPUResourceManager *resources);
        void set_arena(ArenaResource *pmr);

        void image_barrier(const ImageBarrier &barrier) const;
        void image_barriers(std::span<const ImageBarrier> barriers) const;

        void buffer_barrier(const BufferBarrier &barrier) const;
        void buffer_barriers(std::span<const BufferBarrier> barriers) const;

        void begin_rendering(const RenderingInfo &info) const;
        void end_rendering() const;

        void bind_graphics_pipeline(GraphicsPipelineHandle pipeline);
        void bind_compute_pipeline(ComputePipelineHandle pipeline);

        void set_viewport(f32 x, f32 y, f32 width, f32 height) const;
        void set_scissor(i32 x, i32 y, u32 width, u32 height) const;

        void draw(const DrawInfo &info) const;
        void draw_indexed(const DrawIndexedInfo &info) const;

        void dispatch(const DispatchInfo &info) const;
        void copy_buffer(const BufferCopyInfo &info) const;
        void copy_buffer_to_image(const BufferImageCopyInfo &info) const;
        void copy_image(const ImageCopyInfo &info) const;
        void blit_image(const ImageBlitInfo &info) const;
        void copy_image_to_buffer(const ImageBufferCopyInfo &info) const;

        void clear_color_image(ImageHandle image, f32 r, f32 g, f32 b, f32 a) const;
        void clear_depth_image(ImageHandle image, f32 depth) const;

        void bind_vertex_buffer(BufferHandle buffer, u32 binding, usize offset = 0) const;
        void bind_index_buffer(BufferHandle buffer, usize offset = 0) const;

        void push_constants(const void *data, ShaderStage stage) const;

      private:
        VkCommandBuffer m_cmd = VK_NULL_HANDLE;
        GPUResourceManager *m_resources = nullptr;

        ArenaResource *m_pmr;

        VkPipelineLayout m_current_layout = VK_NULL_HANDLE;
        std::span<const PushConstantsRange> m_push_constants;
    };
} // namespace mantle
