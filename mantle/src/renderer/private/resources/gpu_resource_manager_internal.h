#pragma once

#include <vulkan/vulkan.h>

#include "core/memory/pmr/persistent_resource.h"
#include "deletion_queue.h"
#include "renderer/gpu_resource_manager.h"
#include "vulkan_gpu_allocator.h"

namespace mantle {
    template <typename T>
    struct Slot final {
        T resource;
        u32 generation = 0;
    };

    struct BufferResource final {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        void *mapped = nullptr;
        BufferDesc desc = {};

        u32 bindless_index = UINT32_MAX;
    };
    struct ImageResource final {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        ImageDesc desc = {};

        u32 bindless_sample_index = UINT32_MAX;
        u32 bindless_storage_index = UINT32_MAX;
    };
    struct SamplerResource final {
        VkSampler sampler = VK_NULL_HANDLE;
        SamplerDesc desc = {};

        u32 bindless_index = UINT32_MAX;
    };
    struct ShaderResource final {
        VkShaderModule shader = VK_NULL_HANDLE;
    };
    struct GraphicsPipelineResource final {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        GraphicsPipelineDesc desc = {};
    };
    struct ComputePipelineResource final {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        ComputePipelineDesc desc = {};
    };

    struct GPUResourceManager::Impl final {
        static constexpr u32 frame_lag = 3;

        ImageResource &get_image(ImageHandle handle);
        BufferResource &get_buffer(BufferHandle handle);
        SamplerResource &get_sampler(SamplerHandle handle);
        ShaderResource &get_shader(ShaderHandle handle);
        GraphicsPipelineResource &get_graphics_pipeline(GraphicsPipelineHandle handle);
        ComputePipelineResource &get_compute_pipeline(ComputePipelineHandle handle);

        VkDescriptorSet get_bindless_set();

        u32 allocate_storage_image_index(ImageResource &image);
        u32 allocate_sampled_image_index(ImageResource &image);
        u32 allocate_buffer_index(BufferResource &buffer);
        u32 allocate_sampler_index(SamplerResource &sampler);

        void free_storage_image_index(u32 index);
        void free_sampled_image_index(u32 index);
        void free_buffer_index(u32 index);
        void free_sampler_index(u32 index);

        void next_frame();

        VulkanBackend *backend = nullptr;
        VulkanGPUAllocator gpu_allocator{};

        PersistentResource resource;

        std::array<DeletionQueue, frame_lag> deletion_queues;
        u32 current_frame = 0;

        std::pmr::vector<Slot<BufferResource>> buffers;
        std::pmr::vector<u32> buffers_free_list;

        std::pmr::vector<Slot<ImageResource>> images;
        std::pmr::vector<u32> images_free_list;

        std::pmr::vector<Slot<SamplerResource>> samplers;
        std::pmr::vector<u32> samplers_free_list;

        std::pmr::vector<Slot<ShaderResource>> shaders;
        std::pmr::vector<u32> shaders_free_list;

        std::pmr::vector<Slot<GraphicsPipelineResource>> graphics_pipelines;
        std::pmr::vector<u32> graphics_pipelines_free_list;

        std::pmr::vector<Slot<ComputePipelineResource>> compute_pipelines;
        std::pmr::vector<u32> compute_pipelines_free_list;

        u32 sampled_images_count_bindless = 0;
        std::pmr::vector<u32> sampled_images_free_list_bindless;

        u32 storage_images_count_bindless = 0;
        std::pmr::vector<u32> storage_images_free_list_bindless;

        u32 buffers_count_bindless = 0;
        std::pmr::vector<u32> buffers_free_list_bindless;

        u32 samplers_count_bindless = 0;
        std::pmr::vector<u32> samplers_free_list_bindless;

        VkDescriptorPool m_bindless_pool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_bindless_layout = VK_NULL_HANDLE;
        VkDescriptorSet m_bindless = VK_NULL_HANDLE;
    };
} // namespace mantle
