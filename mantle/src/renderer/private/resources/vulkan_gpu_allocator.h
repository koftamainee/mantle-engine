// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "mantle/core/macros.h"
#include "mantle/core/types.h"

namespace spdlog {
    class logger;
}

namespace mantle {
    class VulkanGPUAllocator final {
      public:
        MANTLE_DEFAULT_INIT(VulkanGPUAllocator);

        void init(VkPhysicalDevice physical_device, VkDevice device, VkInstance instance,
                  VkAllocationCallbacks *vk_callbacks);
        void destroy();

        VkResult create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                               VmaMemoryUsage memory_usage, VkBuffer *buffer,
                               VmaAllocation *allocation, void **mapped_data = nullptr) const;

        void destroy_buffer(VkBuffer buffer, VmaAllocation allocation) const;

        VkResult map_memory(VmaAllocation allocation, void **data) const;
        void     unmap_memory(VmaAllocation allocation) const;


        VkResult create_image(const VkImageCreateInfo &image_info, VmaMemoryUsage memory_usage,
                              VkImage *image, VmaAllocation *allocation) const;

        void destroy_image(VkImage image, VmaAllocation allocation) const;


        VkDeviceMemory get_allocation_memory(VmaAllocation allocation) const;
        VkDeviceSize   get_allocation_size(VmaAllocation allocation) const;

      private:
        VmaAllocator m_allocator {};
        bool         m_is_initialized = false;

        spdlog::logger *m_logger = nullptr;
    };
} // namespace mantle
