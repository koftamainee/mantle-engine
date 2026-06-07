#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include "vulkan_gpu_allocator.h"

#include "../vulkan/vkassert.h"
#include "core/assert.h"

namespace mantle {
    VulkanGPUAllocator::~VulkanGPUAllocator() { destroy(); }

    void VulkanGPUAllocator::init(VkPhysicalDevice physical_device,
                                  VkDevice device, VkInstance instance,
                                  VkAllocationCallbacks *vk_callbacks) {
        MANTLE_CHECK(!m_is_initialized);

        m_logger = spdlog::get("renderer").get();

        auto load = [instance](const char *name) {
            return vkGetInstanceProcAddr(instance, name);
        };

        VmaVulkanFunctions vk_fns = {};
        vk_fns.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vk_fns.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
        vk_fns.vkGetPhysicalDeviceProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
                load("vkGetPhysicalDeviceProperties"));
        vk_fns.vkGetPhysicalDeviceMemoryProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(
                load("vkGetPhysicalDeviceMemoryProperties"));
        vk_fns.vkAllocateMemory =
            reinterpret_cast<PFN_vkAllocateMemory>(load("vkAllocateMemory"));
        vk_fns.vkFreeMemory =
            reinterpret_cast<PFN_vkFreeMemory>(load("vkFreeMemory"));
        vk_fns.vkMapMemory =
            reinterpret_cast<PFN_vkMapMemory>(load("vkMapMemory"));
        vk_fns.vkUnmapMemory =
            reinterpret_cast<PFN_vkUnmapMemory>(load("vkUnmapMemory"));
        vk_fns.vkFlushMappedMemoryRanges =
            reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(
                load("vkFlushMappedMemoryRanges"));
        vk_fns.vkInvalidateMappedMemoryRanges =
            reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(
                load("vkInvalidateMappedMemoryRanges"));
        vk_fns.vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(
            load("vkBindBufferMemory"));
        vk_fns.vkBindImageMemory =
            reinterpret_cast<PFN_vkBindImageMemory>(load("vkBindImageMemory"));
        vk_fns.vkGetBufferMemoryRequirements =
            reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(
                load("vkGetBufferMemoryRequirements"));
        vk_fns.vkGetImageMemoryRequirements =
            reinterpret_cast<PFN_vkGetImageMemoryRequirements>(
                load("vkGetImageMemoryRequirements"));
        vk_fns.vkCreateBuffer =
            reinterpret_cast<PFN_vkCreateBuffer>(load("vkCreateBuffer"));
        vk_fns.vkDestroyBuffer =
            reinterpret_cast<PFN_vkDestroyBuffer>(load("vkDestroyBuffer"));
        vk_fns.vkCreateImage =
            reinterpret_cast<PFN_vkCreateImage>(load("vkCreateImage"));
        vk_fns.vkDestroyImage =
            reinterpret_cast<PFN_vkDestroyImage>(load("vkDestroyImage"));
        vk_fns.vkCmdCopyBuffer =
            reinterpret_cast<PFN_vkCmdCopyBuffer>(load("vkCmdCopyBuffer"));
        vk_fns.vkGetBufferMemoryRequirements2KHR =
            reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(
                load("vkGetBufferMemoryRequirements2"));
        vk_fns.vkGetImageMemoryRequirements2KHR =
            reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(
                load("vkGetImageMemoryRequirements2"));
        vk_fns.vkBindBufferMemory2KHR =
            reinterpret_cast<PFN_vkBindBufferMemory2KHR>(
                load("vkBindBufferMemory2"));
        vk_fns.vkBindImageMemory2KHR =
            reinterpret_cast<PFN_vkBindImageMemory2KHR>(
                load("vkBindImageMemory2"));
        vk_fns.vkGetPhysicalDeviceMemoryProperties2KHR =
            reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>(
                load("vkGetPhysicalDeviceMemoryProperties2"));

        VmaAllocatorCreateInfo create_info = {
            .physicalDevice = physical_device,
            .device = device,
            .pAllocationCallbacks = vk_callbacks,
            .pVulkanFunctions = &vk_fns,
            .instance = instance,
        };

        create_info.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

        MANTLE_VK_VERIFY(vmaCreateAllocator(&create_info, &m_allocator));

        m_is_initialized = true;
        m_logger->info("Vulkan memory allocator initialized");
    } // namespace mantle

    void VulkanGPUAllocator::destroy() {
        if (m_is_initialized) {
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
            m_is_initialized = false;
            m_logger->info("Vulkan memory allocator destroyed");
        }
    }

    VkResult VulkanGPUAllocator::create_buffer(VkDeviceSize size,
                                               VkBufferUsageFlags usage,
                                               VmaMemoryUsage memory_usage,
                                               VkBuffer *buffer,
                                               VmaAllocation *allocation,
                                               void **mapped_data) const {
        MANTLE_CHECK(m_is_initialized);
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
        MANTLE_CHECK(m_is_initialized);
        vmaDestroyBuffer(m_allocator, buffer, allocation);
    }

    VkResult VulkanGPUAllocator::map_memory(VmaAllocation allocation,
                                            void **data) const {
        MANTLE_CHECK(m_is_initialized);
        return vmaMapMemory(m_allocator, allocation, data);
    }

    void VulkanGPUAllocator::unmap_memory(VmaAllocation allocation) const {
        MANTLE_CHECK(m_is_initialized);
        vmaUnmapMemory(m_allocator, allocation);
    }

    VkResult VulkanGPUAllocator::create_image(
        const VkImageCreateInfo &image_info, VmaMemoryUsage memory_usage,
        VkImage *image, VmaAllocation *allocation) const {
        MANTLE_CHECK(m_is_initialized);

        VmaAllocationCreateInfo alloc_info = {
            .usage = memory_usage,
        };

        return vmaCreateImage(m_allocator, &image_info, &alloc_info, image,
                              allocation, nullptr);
    }

    void VulkanGPUAllocator::destroy_image(VkImage image,
                                           VmaAllocation allocation) const {
        MANTLE_CHECK(m_is_initialized);
        vmaDestroyImage(m_allocator, image, allocation);
    }

    VkDeviceMemory
    VulkanGPUAllocator::get_allocation_memory(VmaAllocation allocation) const {
        MANTLE_CHECK(m_is_initialized);
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
