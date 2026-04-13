#pragma once

#include <vulkan/vulkan.h>

#include "core/memory/persistent_allocator.h"
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

        void* mapped = nullptr;
    };


    struct GPUResourceManager::Impl final {
        static constexpr u32 frame_lag = 3;

        VkImage get_vk_image(ImageHandle handle) const;
        VkImageView get_vk_image_view(ImageHandle handle) const;
        VkBuffer get_vk_buffer(BufferHandle handle) const;
        VkPipeline get_vk_pipeline(GraphicsPipelineHandle handle) const;
        VkPipeline get_vk_pipeline(ComputePipelineHandle handle) const;
        VkPipelineLayout
        get_vk_pipeline_layout(GraphicsPipelineHandle handle) const;
        VkPipelineLayout
        get_vk_pipeline_layout(ComputePipelineHandle handle) const;

        void next_frame();


        VulkanBackend *backend = nullptr;
        VulkanGPUAllocator gpu_allocator{};

        PersistentResource resource;

        std::array<DeletionQueue, frame_lag> deletion_queues;
        u32 current_frame = 0;

        std::pmr::vector<Slot<BufferResource>> buffers;
        std::pmr::vector<u32> buffers_free_list;
    };
} // namespace mantle
