#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace mantle {

    struct QueueFamilyIndices {
        uint32_t graphics_family = UINT32_MAX;
        uint32_t present_family = UINT32_MAX;
        uint32_t transfer_family = UINT32_MAX;

        bool is_complete() const {
            return graphics_family != UINT32_MAX
                && present_family != UINT32_MAX
                && transfer_family != UINT32_MAX;
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

} // namespace mantle
