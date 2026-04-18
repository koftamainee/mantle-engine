#pragma once
#include <vector>

#include <span>

#include "core/macros.h"
#include "core/types.h"
#include "renderer/types.h"

namespace mantle {
    struct SwapchainInfo;
    class VulkanBackend;

    class GPUResourceManager final {
      public:
        ~GPUResourceManager();

        MANTLE_NO_COPY_NO_MOVE(GPUResourceManager);

        ShaderHandle create_shader(std::span<const u32> spir_v);
        void destroy_shader(ShaderHandle handle, bool immediate = false);

        GraphicsPipelineHandle
        create_graphics_pipeline(const GraphicsPipelineDesc &desc);
        ComputePipelineHandle
        create_compute_pipeline(const ComputePipelineDesc &desc);
        void destroy_graphics_pipeline(GraphicsPipelineHandle handle,
                                       bool immediate = false);
        void destroy_compute_pipeline(ComputePipelineHandle handle,
                                      bool immediate = false);

        BufferHandle create_buffer(const BufferDesc &desc, bool map = false);
        void update_buffer(BufferHandle handle, const void *data, usize size,
                           usize offset = 0);
        void destroy_buffer(BufferHandle handle, bool immediate = false);

        ImageHandle create_image(const ImageDesc &desc);
        void destroy_image(ImageHandle handle, bool immediate = false);

        SamplerHandle create_sampler(const SamplerDesc &desc);
        void destroy_sampler(SamplerHandle handle, bool immediate = false);

        u32 get_bindless_index(ImageHandle handle, BindlessImageType type);
        u32 get_bindless_index(BufferHandle handle);
        u32 get_bindless_index(SamplerHandle handle);

        void free_image_index(ImageHandle handle, BindlessImageType type);
        void free_buffer_index(BufferHandle handle);
        void free_sampler_index(SamplerHandle handle);

      private:
        friend class Renderer;
        friend class CommandRecorder;
        friend class RenderPassContext;

        void import_swapchain_images(std::pmr::vector<ImageHandle> &out_images);
        void release_swapchain_images(std::pmr::vector<ImageHandle> &images);

        static constexpr u32 max_sampled_images = 4096;
        static constexpr u32 max_storage_images = 1024;
        static constexpr u32 max_storage_buffers = 1024;
        static constexpr u32 max_samplers = 256;

        static constexpr u32 sampled_image_binding = 0;
        static constexpr u32 storage_image_binding = 1;
        static constexpr u32 storage_buffer_binding = 2;
        static constexpr u32 sampler_binding = 3;

        void init_bindless();
        void destroy_bindless();

        GPUResourceManager() = default;

        void init(VulkanBackend *backend);
        void destroy();
        struct Impl;

        bool m_is_initialized = false;
        Impl *m_impl = nullptr;
    };
} // namespace mantle
