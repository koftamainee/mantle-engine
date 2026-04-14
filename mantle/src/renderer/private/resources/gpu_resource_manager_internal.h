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
    };
    struct ImageResource final {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        ImageDesc desc = {};
    };
    struct SamplerResource final {
        VkSampler sampler = VK_NULL_HANDLE;
        SamplerDesc desc = {};
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
    };
} // namespace mantle
