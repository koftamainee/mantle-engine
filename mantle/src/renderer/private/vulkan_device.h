#pragma once

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan_types.h"

namespace mantle {
    class VulkanDevice final {
    public:
        VulkanDevice() = default;
        ~VulkanDevice();

        VulkanDevice(const VulkanDevice &) = delete;
        VulkanDevice &operator=(const VulkanDevice &) = delete;
        VulkanDevice(VulkanDevice &&) noexcept = delete;
        VulkanDevice &operator=(VulkanDevice &&) noexcept = delete;

        void init(VkInstance instance, VkSurfaceKHR surface);
        void destroy();

        VkDevice get_device() const;
        VkPhysicalDevice get_physical_device() const;
        SwapchainSupportDetails get_swapchain_support_details(VkSurfaceKHR surface) const;

        uint32_t get_queue_family_index(VkQueueFlags queue_flags) const;
        std::optional<uint32_t> get_memory_type(uint32_t type_bits, VkMemoryPropertyFlags properties) const;
        QueueFamilyIndices get_queue_families() const;

        VkResult copy_buffer(VkBuffer src, VkBuffer dst, VkQueue queue, VkDeviceSize size, VkDeviceSize src_offset = 0,
                             VkDeviceSize dst_offset = 0) const;

        VkCommandPool create_command_pool(uint32_t queue_family_index,
                                          VkCommandPoolCreateFlags create_flags =
                                              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;

        VkCommandBuffer create_command_buffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false) const;
        VkCommandBuffer create_command_buffer(VkCommandBufferLevel level, bool begin = false) const;

        void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool pool,
                                  bool free = true) const;
        void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, bool free = true) const;

        VkCommandBuffer begin_single_time_commands(VkCommandPool pool) const;
        void end_single_time_commands(VkCommandBuffer command_buffer,
                                     VkQueue queue,
                                     VkCommandPool pool) const;

        VkCommandBuffer begin_single_time_commands() const;
        void end_single_time_commands(VkCommandBuffer command_buffer,
                                     VkQueue queue) const;

        bool extension_supported(std::string_view extension) const;
        VkFormat get_supported_depth_format(bool check_sampling_support) const;

    private:
        void create_physical_device(VkInstance instance, VkSurfaceKHR surface);
        void destroy_physical_device();

        void create_logical_device(VkInstance instance);
        void destroy_logical_device();

    private:
        static bool is_physical_device_suitable(
            VkPhysicalDevice physical_device,
            VkSurfaceKHR surface,
            QueueFamilyIndices &queue_family_indices
            );

        static bool is_physical_device_supports_required_extensions(VkPhysicalDevice physical_device);

        static QueueFamilyIndices find_queue_families(
            VkPhysicalDevice physical_device,
            VkSurfaceKHR surface
            );

    private:
        bool m_is_initialized = false;

        VkPhysicalDevice m_physical_device{};
        VkDevice m_device{};
        VkPhysicalDeviceProperties m_properties{};
        VkPhysicalDeviceFeatures m_features{};
        VkPhysicalDeviceFeatures m_enabled_features{};
        VkPhysicalDeviceMemoryProperties m_memory_properties{};
        std::vector<VkQueueFamilyProperties> m_queue_family_properties{};
        std::vector<std::string> m_supported_extensions{};
        VkCommandPool m_command_pool{};
        QueueFamilyIndices m_queue_indices{};

    private:
        inline static const std::vector<const char *> ms_device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifndef NDEBUG
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, // this is for vma debugging
#endif
        };
    };
} // namespace mantle
