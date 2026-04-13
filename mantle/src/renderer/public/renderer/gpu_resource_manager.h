#pragma once
#include <vector>

#include <span>
#include "core/types.h"
#include "renderer/types.h"

namespace mantle {
    struct SwapchainInfo;
    class VulkanBackend;

    class GPUResourceManager final {
      public:
        ~GPUResourceManager();

        GPUResourceManager(const GPUResourceManager &other) = delete;
        GPUResourceManager(GPUResourceManager &&other) noexcept = delete;
        GPUResourceManager &operator=(const GPUResourceManager &other) = delete;
        GPUResourceManager &
        operator=(GPUResourceManager &&other) noexcept = delete;

        ShaderHandle create_shader(std::span<const u32> spir_v);
        void destroy_shader(ShaderHandle shader);

        GraphicsPipelineHandle
        create_graphics_pipeline(const GraphicsPipelineDesc &desc);
        ComputePipelineHandle
        create_compute_pipeline(const ComputePipelineDesc &desc);
        void destroy_graphics_pipeline(GraphicsPipelineHandle pipeline);
        void destroy_compute_pipeline(ComputePipelineHandle pipeline);

        BufferHandle create_buffer(const BufferDesc &desc, bool map = false);
        void update_buffer(BufferHandle handle, const void *data, usize size,
                           usize offset = 0);
        void destroy_buffer(BufferHandle handle);

        ImageHandle create_image(const ImageDesc &desc);
        void destroy_image(ImageHandle handle);

        SamplerHandle create_sampler(const SamplerDesc &desc);
        void destroy_sampler(SamplerHandle sampler);

        u32 get_bindless_index(ImageHandle image);
        u32 get_bindless_index(BufferHandle buffer);
        u32 get_bindless_index(SamplerHandle sampler);

      private:
        friend class Renderer;
        friend class CommandRecorder;
        void import_swapchain_images(std::pmr::vector<ImageHandle> &out_images);
        void release_swapchain_images(std::pmr::vector<ImageHandle> &images);

        GPUResourceManager() = default;

        void init(VulkanBackend *backend);
        void destroy();
        struct Impl;

        bool m_is_initialized = false;
        Impl *m_impl = nullptr;
    };
} // namespace mantle
