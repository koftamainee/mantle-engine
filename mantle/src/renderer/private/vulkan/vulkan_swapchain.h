// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <vulkan/vulkan.h>

#include <span>
#include <vector>

#include "mantle/core/macros.h"
#include "mantle/core/memory/arena_allocator.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/memory/pmr/arena_resource.h"
#include "mantle/core/memory/pmr/tlsf_resource.h"
#include "mantle/core/memory/tlsf_allocator.h"
#include "mantle/core/types.h"
#include "vulkan_types.h"

namespace spdlog {
    class logger;
}

namespace mantle {

    class VulkanSwapchain final {
      public:
        MANTLE_DEFAULT_INIT(VulkanSwapchain);

        struct Image {
            VkImage     image;
            VkImageView view;
        };

        void init(VkDevice device, VkSurfaceKHR surface,
                  const SwapchainSupportDetails &support_details, const QueueFamilyIndices &indices,
                  u32 width, u32 height, bool vsync, VkAllocationCallbacks *vk_callbacks,
                  TlsfAllocator *allocator, ArenaAllocator *scratch_arena);

        void destroy();

        std::span<const Image> get_images() const;
        VkSwapchainKHR         get_swapchain() const;
        VkExtent2D             get_extent() const;
        VkSurfaceFormatKHR     get_surface_format() const;

      private:
        VkSurfaceFormatKHR pick_surface_format(std::span<const VkSurfaceFormatKHR> formats) const;
        VkExtent2D         pick_extent(const VkSurfaceCapabilitiesKHR &capabilities, u32 width,
                                       u32 height) const;
        VkPresentModeKHR   pick_present_mode(std::span<const VkPresentModeKHR> present_modes,
                                             bool                              vsync) const;

      private:
        bool m_is_initialized = false;

        VkAllocationCallbacks *m_alloc_callbacks = nullptr;

        VkSwapchainKHR          m_swapchain = VK_NULL_HANDLE;
        std::pmr::vector<Image> m_images {};

        ArenaAllocator *m_scratch_arena = nullptr;
        ArenaResource   m_scratch_resource {};

        TlsfAllocator *m_allocator = nullptr;
        TlsfResource   m_tlsf_resource {};

        VkDevice           m_device = VK_NULL_HANDLE;
        VkSurfaceFormatKHR m_surface_format {};
        VkExtent2D         m_extent {};
        VkPresentModeKHR   m_present_mode {};

        spdlog::logger *m_logger = nullptr;
    };

} // namespace mantle
