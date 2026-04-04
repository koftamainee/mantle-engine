#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan_types.h"

namespace mantle {
  class VulkanDevice final {
  public:
    VulkanDevice() = default;
    ~VulkanDevice();

    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;
    VulkanDevice(VulkanDevice&&) noexcept = delete;
    VulkanDevice& operator=(VulkanDevice&&) noexcept = delete;

    void init(VkInstance instance, VkSurfaceKHR surface);
    void destroy();

    VkDevice get_device() const;
    SwapchainSupportDetails get_swapchain_support_details(VkSurfaceKHR surface) const;
    QueueFamilyIndices get_queue_families() const;

  private:
    void create_physical_device(VkInstance instance, VkSurfaceKHR surface);
    void destroy_physical_device();

    void create_logical_device(VkInstance instance);
    void destroy_logical_device();

    QueueFamilyIndices queue_indices{};

  private:
    static bool is_physical_device_suitable(
      VkPhysicalDevice physical_device,
      VkSurfaceKHR surface,
      QueueFamilyIndices& queue_family_indices
    );

    static bool is_physical_device_supports_required_extensions(VkPhysicalDevice physical_device);

    static QueueFamilyIndices find_queue_families(
      VkPhysicalDevice physical_device,
      VkSurfaceKHR surface
    );

  private:
    bool m_is_initialized = false;

    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;

  private:
    inline static const std::vector<const char*> ms_device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
    };
  };
} // namespace mantle
