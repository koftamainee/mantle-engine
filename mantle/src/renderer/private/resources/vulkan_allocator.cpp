#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include "vulkan_allocator.h"

#include "../vulkan/vkassert.h"
#include "core/assert.h"

namespace mantle {
    VulkanAllocator::~VulkanAllocator() {
        destroy();
    }

    void VulkanAllocator::init(VkPhysicalDevice physical_device, VkDevice device, VkInstance instance) {
        check(!m_is_initialized);

        VmaAllocatorCreateInfo create_info = {
            .physicalDevice = physical_device,
            .device = device,
            .instance = instance,
        };

#ifndef NDEBUG
        create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
            VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
#endif

        vk_verify(vmaCreateAllocator(&create_info, &m_allocator));

        m_is_initialized = true;
        spdlog::info("VulkanMemoryAllocator is initialized");
    }

    void VulkanAllocator::destroy() {
        if (m_is_initialized) {
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
            m_is_initialized = false;
            spdlog::info("VulkanMemoryAllocator is destroyed");
        }
    }

    VkResult VulkanAllocator::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage,
                                            VkBuffer *buffer, VmaAllocation *allocation, void **mapped_data) const {
        check(m_is_initialized);
        VkBufferCreateInfo buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
        };

        VmaAllocationCreateInfo alloc_info = {
            .usage = memory_usage,
        };
        if (mapped_data != nullptr) {
            alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        VmaAllocationInfo alloc_info_out = {};

        VkResult result = vmaCreateBuffer(m_allocator, &buffer_info, &alloc_info, buffer, allocation, &alloc_info_out);

        if (result != VK_SUCCESS)
            return result;

        if (mapped_data && alloc_info_out.pMappedData) {
            *mapped_data = alloc_info_out.pMappedData;
        }

        return VK_SUCCESS;
    }

    void VulkanAllocator::destroy_buffer(VkBuffer buffer, VmaAllocation allocation) const {
        check(m_is_initialized);
        vmaDestroyBuffer(m_allocator, buffer, allocation);
    }

    VkResult VulkanAllocator::map_memory(VmaAllocation allocation, void **data) const {
        check(m_is_initialized);
        return vmaMapMemory(m_allocator, allocation, data);
    }

    void VulkanAllocator::unmap_memory(VmaAllocation allocation) const {
        check(m_is_initialized);
        vmaUnmapMemory(m_allocator, allocation);
    }

    VkResult VulkanAllocator::create_image(const VkImageCreateInfo &image_info, VmaMemoryUsage memory_usage,
                                           VkImage *image, VmaAllocation *allocation) const {
        check(m_is_initialized);

        VmaAllocationCreateInfo alloc_info = {
            .usage = memory_usage,
        };

        return vmaCreateImage(m_allocator, &image_info, &alloc_info, image, allocation, nullptr);
    }

    void VulkanAllocator::destroy_image(VkImage image, VmaAllocation allocation) const {
        check(m_is_initialized);
        vmaDestroyImage(m_allocator, image, allocation);
    }

    VkDeviceMemory VulkanAllocator::get_allocation_memory(VmaAllocation allocation) const {
        check(m_is_initialized);
        VmaAllocationInfo info = {};
        vmaGetAllocationInfo(m_allocator, allocation, &info);
        return info.deviceMemory;
    }

    VkDeviceSize VulkanAllocator::get_allocation_size(VmaAllocation allocation) const {
        VmaAllocationInfo info = {};
        vmaGetAllocationInfo(m_allocator, allocation, &info);
        return info.size;
    }
}
