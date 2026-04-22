#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include "core/macros.h"
#include "vulkan_types.h"

#include "core/memory/arena_allocator.h"
#include "core/memory/persistent_allocator.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/memory/pmr/persistent_resource.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"

namespace mantle {

    class VulkanSwapchain final {
      public:
        VulkanSwapchain() = default;
        ~VulkanSwapchain();

        MANTLE_NO_COPY_NO_MOVE(VulkanSwapchain);

        struct Image {
            VkImage image;
            VkImageView view;
        };

        void init(VkDevice device, VkSurfaceKHR surface,
                  const SwapchainSupportDetails &support_details,
                  const QueueFamilyIndices &indices, u32 width, u32 height,
                  bool vsync, VkAllocationCallbacks *vk_callbacks, VirtualHeap *heap, ArenaAllocator *scratch_arena);

        void destroy();

        std::span<const Image> get_images() const;
        VkSwapchainKHR get_swapchain() const;
        VkExtent2D get_extent() const;
        VkSurfaceFormatKHR get_surface_format() const;

      private:
        static VkSurfaceFormatKHR
        pick_surface_format(std::span<const VkSurfaceFormatKHR> formats);
        static VkExtent2D
        pick_extent(const VkSurfaceCapabilitiesKHR &capabilities, u32 width,
                    u32 height);
        static VkPresentModeKHR
        pick_present_mode(std::span<const VkPresentModeKHR> present_modes,
                          bool vsync);

      private:
        bool m_is_initialized = false;

        VkAllocationCallbacks *m_alloc_callbacks = nullptr;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        std::pmr::vector<Image> m_images{};

        ArenaAllocator *m_scratch_arena = nullptr;
        ArenaResource m_scratch_resource{};

        VirtualHeap *m_heap = nullptr;

        PersistentAllocator m_persistent_allocator{};
        PersistentResource m_persistent_resource{};

        VkDevice m_device = VK_NULL_HANDLE;
        VkSurfaceFormatKHR m_surface_format{};
        VkExtent2D m_extent{};
        VkPresentModeKHR m_present_mode{};
    };

} // namespace mantle
