// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "mantle/core/types.h"

namespace mantle {

    struct QueueFamilyIndices {
        u32 graphics_family = UINT32_MAX;
        u32 present_family = UINT32_MAX;
        u32 transfer_family = UINT32_MAX;

        bool is_complete() const {
            return graphics_family != UINT32_MAX && present_family != UINT32_MAX &&
                   transfer_family != UINT32_MAX;
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR             capabilities;
        std::pmr::vector<VkSurfaceFormatKHR> formats;
        std::pmr::vector<VkPresentModeKHR>   present_modes;
    };

} // namespace mantle
