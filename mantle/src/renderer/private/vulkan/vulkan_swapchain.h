#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include "vulkan_types.h"

#include "core/types.h"

namespace mantle {

    class VulkanSwapchain final {
      public:
        VulkanSwapchain() = default;
        ~VulkanSwapchain();

        VulkanSwapchain(const VulkanSwapchain &) = delete;
        VulkanSwapchain &operator=(const VulkanSwapchain &) = delete;
        VulkanSwapchain(VulkanSwapchain &&) noexcept = delete;
        VulkanSwapchain &operator=(VulkanSwapchain &&) noexcept = delete;

        struct Image {
            VkImage image;
            VkImageView view;
        };

        void init(VkDevice device, VkSurfaceKHR surface,
                  const SwapchainSupportDetails &support_details,
                  const QueueFamilyIndices &indices, u32 width, u32 height, bool vsync, VkAllocationCallbacks *vk_callbacks);

        void destroy();

        std::vector<Image> get_images() const;
        VkSwapchainKHR get_swapchain() const;
        VkExtent2D get_extent() const;
        VkSurfaceFormatKHR get_surface_format() const;

      private:
        static VkSurfaceFormatKHR
        pick_surface_format(const std::vector<VkSurfaceFormatKHR> &formats);
        static VkExtent2D
        pick_extent(const VkSurfaceCapabilitiesKHR &capabilities, u32 width,
                    u32 height);
        static VkPresentModeKHR
        pick_present_mode(const std::vector<VkPresentModeKHR> &present_modes, bool vsync);

      private:
        bool m_is_initialized = false;

        VkAllocationCallbacks *m_alloc_callbacks = nullptr;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        std::vector<Image> m_images{};

        VkDevice m_device = VK_NULL_HANDLE;
        VkSurfaceFormatKHR m_surface_format{};
        VkExtent2D m_extent{};
        VkPresentModeKHR m_present_mode{};
    };

} // namespace mantle
