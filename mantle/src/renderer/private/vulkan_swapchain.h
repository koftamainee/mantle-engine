#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "vulkan_types.h"
#include <GLFW/glfw3.h>

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

        void init(VkDevice device,
                  VkSurfaceKHR surface,
                  const SwapchainSupportDetails &support_details,
                  const QueueFamilyIndices &indices,
                  uint32_t width,
                  uint32_t height);

        void destroy();

        std::vector<Image> get_images() const;
        VkSwapchainKHR get_swapchain() const;
        VkExtent2D get_extent() const;
        VkSurfaceFormatKHR get_surface_format() const;

    private:
        static VkSurfaceFormatKHR pick_surface_format(const std::vector<VkSurfaceFormatKHR> &formats);
        static VkExtent2D pick_extent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t width, uint32_t height);
        static VkPresentModeKHR pick_present_mode(const std::vector<VkPresentModeKHR> &present_modes);

    private:
        bool m_is_initialized = false;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        std::vector<Image> m_images{};

        VkDevice m_device = VK_NULL_HANDLE;
        VkSurfaceFormatKHR m_surface_format{};
        VkExtent2D m_extent{};
        VkPresentModeKHR m_present_mode{};
    };

} // namespace mantle
