#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include "vulkan_gpu_allocator.h"

#include "../vulkan/vkassert.h"
#include "core/assert.h"

namespace mantle {
    VulkanGPUAllocator::~VulkanGPUAllocator() { destroy(); }

    void VulkanGPUAllocator::init(VkPhysicalDevice physical_device,
                                  VkDevice device, VkInstance instance, VkAllocationCallbacks *vk_callbacks) {
        check(!m_is_initialized);

        VmaAllocatorCreateInfo create_info = {
            .physicalDevice = physical_device,
            .device = device,
            .pAllocationCallbacks = vk_callbacks,
            .instance = instance,
        };

#ifndef NDEBUG
        create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
            VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
#endif

        vk_verify(vmaCreateAllocator(&create_info, &m_allocator));

        m_is_initialized = true;
        spdlog::info("VulkanMemoryAllocator is initialized");
    } // namespace mantle

    void VulkanGPUAllocator::destroy() {
        if (m_is_initialized) {
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
            m_is_initialized = false;
            spdlog::info("VulkanMemoryAllocator is destroyed");
        }
    }

    VkResult VulkanGPUAllocator::create_buffer(VkDeviceSize size,
                                               VkBufferUsageFlags usage,
                                               VmaMemoryUsage memory_usage,
                                               VkBuffer *buffer,
                                               VmaAllocation *allocation,
                                               void **mapped_data) const {
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
            alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }

        VmaAllocationInfo alloc_info_out = {};

        VkResult result =
            vmaCreateBuffer(m_allocator, &buffer_info, &alloc_info, buffer,
                            allocation, &alloc_info_out);

        if (result != VK_SUCCESS)
            return result;

        if (mapped_data && alloc_info_out.pMappedData) {
            *mapped_data = alloc_info_out.pMappedData;
        }

        return VK_SUCCESS;
    }

    void VulkanGPUAllocator::destroy_buffer(VkBuffer buffer,
                                            VmaAllocation allocation) const {
        check(m_is_initialized);
        vmaDestroyBuffer(m_allocator, buffer, allocation);
    }

    VkResult VulkanGPUAllocator::map_memory(VmaAllocation allocation,
                                            void **data) const {
        check(m_is_initialized);
        return vmaMapMemory(m_allocator, allocation, data);
    }

    void VulkanGPUAllocator::unmap_memory(VmaAllocation allocation) const {
        check(m_is_initialized);
        vmaUnmapMemory(m_allocator, allocation);
    }

    VkResult VulkanGPUAllocator::create_image(
        const VkImageCreateInfo &image_info, VmaMemoryUsage memory_usage,
        VkImage *image, VmaAllocation *allocation) const {
        check(m_is_initialized);

        VmaAllocationCreateInfo alloc_info = {
            .usage = memory_usage,
        };

        return vmaCreateImage(m_allocator, &image_info, &alloc_info, image,
                              allocation, nullptr);
    }

    void VulkanGPUAllocator::destroy_image(VkImage image,
                                           VmaAllocation allocation) const {
        check(m_is_initialized);
        vmaDestroyImage(m_allocator, image, allocation);
    }

    VkDeviceMemory
    VulkanGPUAllocator::get_allocation_memory(VmaAllocation allocation) const {
        check(m_is_initialized);
        VmaAllocationInfo info = {};
        vmaGetAllocationInfo(m_allocator, allocation, &info);
        return info.deviceMemory;
    }

    VkDeviceSize
    VulkanGPUAllocator::get_allocation_size(VmaAllocation allocation) const {
        VmaAllocationInfo info = {};
        vmaGetAllocationInfo(m_allocator, allocation, &info);
        return info.size;
    }
} // namespace mantle
