// Copyright (c) 2026 Mantle. All rights reserved.

#include "vulkan/vulkan_swapchain.h"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <array>

#include "mantle/core/assert.h"
#include "mantle/core/memory/scope_arena.h"
#include "vkassert.h"
#include "../vulkan/vulkan_types.h"

namespace mantle {

    void VulkanSwapchain::init(VkDevice device, VkSurfaceKHR surface,
                               const SwapchainSupportDetails &support_details,
                               const QueueFamilyIndices &indices, u32 width, u32 height, bool vsync,
                               VkAllocationCallbacks *vk_callbacks, TlsfAllocator *allocator,
                               ArenaAllocator *scratch_arena) {
        MANTLE_CHECK(!m_is_initialized);
        MANTLE_CHECK(device != VK_NULL_HANDLE);

        m_logger = spdlog::get("vulkan").get();
        m_device = device;
        m_alloc_callbacks = vk_callbacks;

        m_scratch_arena = scratch_arena;
        m_allocator = allocator;
        m_scratch_resource = ArenaResource(m_scratch_arena);

        m_tlsf_resource = TlsfResource(allocator);

        m_images = std::pmr::vector<Image>(&m_tlsf_resource);

        m_surface_format = pick_surface_format(support_details.formats);
        m_extent = pick_extent(support_details.capabilities, width, height);
        m_present_mode = pick_present_mode(support_details.present_modes, vsync);

        u32 image_count = support_details.capabilities.minImageCount + 1;
        if (support_details.capabilities.maxImageCount > 0 &&
            image_count > support_details.capabilities.maxImageCount) {
            image_count = support_details.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .flags = VkSwapchainCreateFlagsKHR(),
            .surface = surface,
            .minImageCount = image_count,
            .imageFormat = m_surface_format.format,
            .imageColorSpace = m_surface_format.colorSpace,
            .imageExtent = m_extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = support_details.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = m_present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
        };

        const std::array indices_array = {indices.graphics_family, indices.present_family};

        if (indices_array[0] != indices_array[1]) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = indices_array.data();
        }

        MANTLE_VK_VERIFY(
            vkCreateSwapchainKHR(device, &create_info, m_alloc_callbacks, &m_swapchain));

        MANTLE_VK_VERIFY(vkGetSwapchainImagesKHR(device, m_swapchain, &image_count, nullptr));
        ScopeArena                scope(m_scratch_arena);
        std::pmr::vector<VkImage> images(image_count, &m_scratch_resource);
        MANTLE_VK_VERIFY(vkGetSwapchainImagesKHR(device, m_swapchain, &image_count, images.data()));

        m_images.resize(image_count);
        for (usize i = 0; i < image_count; i++) {
            m_images[i].image = images[i];

            VkImageViewCreateInfo view_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_images[i].image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_surface_format.format,
                .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                               VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};


            MANTLE_VK_VERIFY(
                vkCreateImageView(m_device, &view_info, m_alloc_callbacks, &m_images[i].view));
        }

        m_is_initialized = true;
        m_logger->info("Swapchain created");
    }

    void VulkanSwapchain::destroy() {
        if (m_is_initialized) {
            MANTLE_CHECK(m_device != VK_NULL_HANDLE);

            for (auto [_, view] : m_images) {
                vkDestroyImageView(m_device, view, m_alloc_callbacks);
            }
            m_images.clear();

            vkDestroySwapchainKHR(m_device, m_swapchain, m_alloc_callbacks);

            m_device = VK_NULL_HANDLE;
            m_is_initialized = false;
            m_logger->info("Swapchain destroyed");
        }
    }

    std::span<const VulkanSwapchain::Image> VulkanSwapchain::get_images() const {
        MANTLE_CHECK(m_is_initialized);

        return m_images;
    }

    VkSwapchainKHR VulkanSwapchain::get_swapchain() const {
        MANTLE_CHECK(m_is_initialized);
        return m_swapchain;
    }

    VkExtent2D VulkanSwapchain::get_extent() const {
        MANTLE_CHECK(m_is_initialized);
        return m_extent;
    }

    VkSurfaceFormatKHR VulkanSwapchain::get_surface_format() const {
        MANTLE_CHECK(m_is_initialized);
        return m_surface_format;
    }

    namespace {
        const char *format_name(VkFormat format) {
            switch (format) {
                case VK_FORMAT_R8G8B8A8_UNORM: {
                    return "R8G8B8A8_UNORM";
                }
                case VK_FORMAT_R8G8B8A8_SRGB: {
                    return "R8G8B8A8_SRGB";
                }
                case VK_FORMAT_B8G8R8A8_UNORM: {
                    return "B8G8R8A8_UNORM";
                }
                case VK_FORMAT_B8G8R8A8_SRGB: {
                    return "B8G8R8A8_SRGB";
                }
                case VK_FORMAT_A8B8G8R8_UNORM_PACK32: {
                    return "A8B8G8R8_UNORM_PACK32";
                }
                case VK_FORMAT_A8B8G8R8_SRGB_PACK32: {
                    return "A8B8G8R8_SRGB_PACK32";
                }
                default: {
                    return "UNKNOWN";
                }
            }
        }

        const char *colorspace_name(VkColorSpaceKHR cs) {
            switch (cs) {
                case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: {
                    return "SRGB_NONLINEAR_KHR";
                }
                default: {
                    return "UNKNOWN";
                }
            }
        }
    } // namespace

    VkSurfaceFormatKHR
    VulkanSwapchain::pick_surface_format(const std::span<const VkSurfaceFormatKHR> formats) const {

        m_logger->info("Available surface formats ({}):", formats.size());
        for (const auto &fmt : formats) {
            m_logger->info("  {} / {}", format_name(fmt.format), colorspace_name(fmt.colorSpace));
        }

        constexpr std::array preferred_formats = {
            VK_FORMAT_R8G8B8A8_SRGB,  VK_FORMAT_B8G8R8A8_SRGB,  VK_FORMAT_A8B8G8R8_SRGB_PACK32,
            VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_A8B8G8R8_UNORM_PACK32,
        };

        for (VkFormat pref : preferred_formats) {
            for (const auto &fmt : formats) {
                if (fmt.format == pref && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    m_logger->info("Chosen surface format: {} / {}", format_name(fmt.format),
                                   colorspace_name(fmt.colorSpace));
                    return fmt;
                }
            }
        }

        m_logger->warn("No preferred surface format found, using first available: {} / {}",
                       format_name(formats[0].format), colorspace_name(formats[0].colorSpace));
        return formats[0];
    }

    VkExtent2D VulkanSwapchain::pick_extent(const VkSurfaceCapabilitiesKHR &capabilities, u32 width,
                                            u32 height) const {
        VkExtent2D extent;

        if (capabilities.currentExtent.width != UINT32_MAX) {
            extent = capabilities.currentExtent;
            m_logger->info("Swapchain extent: {}x{} (fixed)", extent.width, extent.height);
        } else {
            extent = {
                std::clamp<u32>(width, capabilities.minImageExtent.width,
                                capabilities.maxImageExtent.width),
                std::clamp<u32>(height, capabilities.minImageExtent.height,
                                capabilities.maxImageExtent.height),
            };
            m_logger->info("Swapchain extent: {}x{} (clamped [{}-{}]x[{}-{}])", extent.width,
                           extent.height, capabilities.minImageExtent.width,
                           capabilities.maxImageExtent.width, capabilities.minImageExtent.height,
                           capabilities.maxImageExtent.height);
        }

        if (extent.width == 0 || extent.height == 0) {
            extent.width = std::max(extent.width, std::max(width, 1u));
            extent.height = std::max(extent.height, std::max(height, 1u));
            m_logger->warn("Swapchain extent was 0, clamped to {}x{}", extent.width, extent.height);
        }

        return extent;
    }

    VkPresentModeKHR
    VulkanSwapchain::pick_present_mode(std::span<const VkPresentModeKHR> present_modes,
                                       bool                              vsync) const {
        if (vsync) {
            for (const auto &mode : present_modes) {
                if (mode == VK_PRESENT_MODE_FIFO_KHR) {
                    m_logger->info("VSync enabled. Chosen present mode: "
                                   "VK_PRESENT_MODE_FIFO_KHR");
                    return mode;
                }
            }
            m_logger->warn("VK_PRESENT_MODE_FIFO_KHR not found, using first available");
            return present_modes[0];
        }

        for (const auto &mode : present_modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                m_logger->info("Chosen present mode: VK_PRESENT_MODE_MAILBOX_KHR");
                return mode;
            }
        }

        for (const auto &mode : present_modes) {
            if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                m_logger->info("VK_PRESENT_MODE_MAILBOX_KHR not available. "
                               "Using VK_PRESENT_MODE_IMMEDIATE_KHR");
                return mode;
            }
        }

        for (const auto &mode : present_modes) {
            if (mode == VK_PRESENT_MODE_FIFO_KHR) {
                m_logger->warn("VSync is off, but neither MAILBOX nor IMMEDIATE available. "
                               "Fallback to VK_PRESENT_MODE_FIFO_KHR");
                return mode;
            }
        }

        MANTLE_FATAL(true, "Unsupported present mode");
    }

} // namespace mantle
