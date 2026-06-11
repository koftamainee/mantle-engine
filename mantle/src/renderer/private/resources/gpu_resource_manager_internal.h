// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <vulkan/vulkan.h>

#include "mantle/core/memory/pmr/tlsf_resource.h"
#include "deletion_queue.h"
#include "mantle/renderer/gpu_resource_manager.h"
#include "resources.h"
#include "types.h"
#include "vulkan_gpu_allocator.h"

namespace mantle {
    template <typename T>
    struct Slot final {
        T   resource;
        u32 generation = 0;
    };

    struct GPUResourceManager::Impl final {
        static constexpr u32 frame_lag = 3;

        ImageResource            &get_image(ImageHandle handle);
        BufferResource           &get_buffer(BufferHandle handle);
        SamplerResource          &get_sampler(SamplerHandle handle);
        ShaderResource           &get_shader(ShaderHandle handle);
        GraphicsPipelineResource &get_graphics_pipeline(GraphicsPipelineHandle handle);
        ComputePipelineResource  &get_compute_pipeline(ComputePipelineHandle handle);

        VkDescriptorSet get_bindless_set() const;

        u32 allocate_storage_image_index(ImageResource &image);
        u32 allocate_sampled_image_index(ImageResource &image);
        u32 allocate_buffer_index(BufferResource &buffer);
        u32 allocate_sampler_index(SamplerResource &sampler);

        void free_storage_image_index(u32 index);
        void free_sampled_image_index(u32 index);
        void free_buffer_index(u32 index);
        void free_sampler_index(u32 index);

        void next_frame();

        VulkanBackend     *backend = nullptr;
        VulkanGPUAllocator gpu_allocator {};

        TlsfAllocator *allocator = nullptr;
        TlsfResource   resource;

        std::array<DeletionQueue, frame_lag> deletion_queues;
        u32                                  current_frame = 0;

        std::pmr::vector<Slot<BufferResource>> buffers;
        std::pmr::vector<u32>                  buffers_free_list;

        std::pmr::vector<Slot<ImageResource>> images;
        std::pmr::vector<u32>                 images_free_list;

        std::pmr::vector<Slot<SamplerResource>> samplers;
        std::pmr::vector<u32>                   samplers_free_list;

        std::pmr::vector<Slot<ShaderResource>> shaders;
        std::pmr::vector<u32>                  shaders_free_list;

        std::pmr::vector<Slot<GraphicsPipelineResource>> graphics_pipelines;
        std::pmr::vector<u32>                            graphics_pipelines_free_list;

        std::pmr::vector<Slot<ComputePipelineResource>> compute_pipelines;
        std::pmr::vector<u32>                           compute_pipelines_free_list;

        u32                   sampled_images_count_bindless = 0;
        std::pmr::vector<u32> sampled_images_free_list_bindless;

        u32                   storage_images_count_bindless = 0;
        std::pmr::vector<u32> storage_images_free_list_bindless;

        u32                   buffers_count_bindless = 0;
        std::pmr::vector<u32> buffers_free_list_bindless;

        u32                   samplers_count_bindless = 0;
        std::pmr::vector<u32> samplers_free_list_bindless;

        VkDescriptorPool      m_bindless_pool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_bindless_layout = VK_NULL_HANDLE;
        VkDescriptorSet       m_bindless = VK_NULL_HANDLE;
    };
} // namespace mantle
