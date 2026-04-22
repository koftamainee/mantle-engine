#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "core/macros.h"
#include "core/memory/arena_allocator.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/memory/pmr/persistent_resource.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"
#include "vulkan_types.h"

namespace mantle {
    class VulkanDevice final {
    public:
        VulkanDevice() = default;
        ~VulkanDevice();

        MANTLE_NO_COPY_NO_MOVE(VulkanDevice);

        void init(VkInstance instance, VkSurfaceKHR surface,
                  VkAllocationCallbacks *vk_callbacks, VirtualHeap *heap, ArenaAllocator *scratch_arena);
        void destroy();

        VkDevice get_device() const;
        VkPhysicalDevice get_physical_device() const;
        SwapchainSupportDetails
        get_swapchain_support_details(VkSurfaceKHR surface, std::pmr::memory_resource *pmr) const;

        u32 get_queue_family_index(VkQueueFlags queue_flags) const;
        std::optional<u32>
        get_memory_type(u32 type_bits, VkMemoryPropertyFlags properties) const;
        QueueFamilyIndices get_queue_families() const;

        VkResult copy_buffer(VkBuffer src, VkBuffer dst, VkQueue queue,
                             VkDeviceSize size, VkDeviceSize src_offset = 0,
                             VkDeviceSize dst_offset = 0) const;

        VkCommandPool create_command_pool(
            u32 queue_family_index,
            VkCommandPoolCreateFlags create_flags =
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;

        VkCommandBuffer create_command_buffer(VkCommandBufferLevel level,
                                              VkCommandPool pool,
                                              bool begin = false) const;
        VkCommandBuffer create_command_buffer(VkCommandBufferLevel level,
                                              bool begin = false) const;

        void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue,
                                  VkCommandPool pool, bool free = true) const;
        void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue,
                                  bool free = true) const;

        VkCommandBuffer begin_single_time_commands(VkCommandPool pool) const;
        void end_single_time_commands(VkCommandBuffer command_buffer,
                                      VkQueue queue, VkCommandPool pool) const;

        VkCommandBuffer begin_single_time_commands() const;
        void end_single_time_commands(VkCommandBuffer command_buffer,
                                      VkQueue queue) const;

        bool extension_supported(std::string_view extension) const;
        VkFormat get_supported_depth_format(bool check_sampling_support) const;

        VkQueue get_graphics_queue() const;
        VkQueue get_present_queue() const;
        VkQueue get_transfer_queue() const;

    private:
        void create_physical_device(VkInstance instance, VkSurfaceKHR surface);
        void destroy_physical_device();

        void create_logical_device(VkInstance instance);
        void destroy_logical_device();

    private:
        bool
        is_physical_device_suitable(VkPhysicalDevice physical_device,
                                    VkSurfaceKHR surface,
                                    QueueFamilyIndices &queue_family_indices);

        bool is_physical_device_supports_required_extensions(
            VkPhysicalDevice physical_device);

        QueueFamilyIndices
        find_queue_families(VkPhysicalDevice physical_device,
                            VkSurfaceKHR surface);

    private:
        bool m_is_initialized = false;

        VkAllocationCallbacks *m_alloc_callbacks = nullptr;

        ArenaAllocator *m_scratch_arena = nullptr;
        ArenaResource m_scratch_resource{};
        PersistentResource m_resource{};

        VkPhysicalDevice m_physical_device{};
        VkDevice m_device{};
        VkPhysicalDeviceProperties m_properties{};
        VkPhysicalDeviceFeatures m_features{};
        VkPhysicalDeviceFeatures m_enabled_features{};
        VkPhysicalDeviceMemoryProperties m_memory_properties{};
        std::pmr::vector<VkQueueFamilyProperties> m_queue_family_properties{};
        std::pmr::vector<std::pmr::string> m_supported_extensions{};
        VkCommandPool m_command_pool{};
        VkCommandPool m_transfer_command_pool{};
        QueueFamilyIndices m_queue_indices{};

        VkQueue m_graphics_queue{};
        VkQueue m_present_queue{};
        VkQueue m_transfer_queue{};


      private:
#ifndef NDEBUG
        static constexpr std::array<const char *, 2> ms_device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
        };
#else
        static constexpr std::array<const char *, 1> ms_device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
#endif
    };
} // namespace mantle
