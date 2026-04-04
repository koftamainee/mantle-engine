#include "vulkan_device.h"

#include <core/assert.h>

#include <vkassert.h>
#include "vulkan_types.h"
#include <cstring>
#include <unordered_set>

#include <spdlog/spdlog.h>

#include "vkassert.h"

namespace mantle {

VulkanDevice::~VulkanDevice() { destroy(); }

void VulkanDevice::init(VkInstance instance, VkSurfaceKHR surface) {
    check(!m_is_initialized);

    create_physical_device(instance, surface);
    create_logical_device(instance);

    m_is_initialized = true;
}

void VulkanDevice::destroy() {
    if (m_is_initialized) {
        destroy_logical_device();
        destroy_physical_device();
    }
}

VkDevice VulkanDevice::get_device() const {
    check(m_is_initialized);
    return m_device;
}

SwapchainSupportDetails VulkanDevice::get_swapchain_support_details(VkSurfaceKHR surface) const {
    check(m_is_initialized);
    check(surface != VK_NULL_HANDLE);
    check(m_physical_device != VK_NULL_HANDLE);

    SwapchainSupportDetails details;

    vk_verify(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, surface, &details.capabilities));

    uint32_t format_count = 0;
    vk_verify(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, surface, &format_count, nullptr));

    details.formats.resize(format_count);
    vk_verify(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, surface, &format_count, details.formats.data()));

    uint32_t present_mode_count = 0;
    vk_verify(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, surface, &present_mode_count, nullptr));

    details.present_modes.resize(present_mode_count);
    vk_verify(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, surface, &present_mode_count, details.present_modes.data()));

    return details;
}

QueueFamilyIndices VulkanDevice::get_queue_families() const {
    check(m_is_initialized);
    return queue_indices;
}

void VulkanDevice::create_physical_device(VkInstance instance, VkSurfaceKHR surface) {
    check(instance != VK_NULL_HANDLE);

    uint32_t device_count = 0;
    vk_verify(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
    fatal(device_count == 0, "Failed to enumerate physical devices");

    std::vector<VkPhysicalDevice> devices(device_count);
    vk_verify(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()));

    for (const auto& physical_device_candidate : devices) {
        if (is_physical_device_suitable(physical_device_candidate, surface, queue_indices)) {
            m_physical_device = physical_device_candidate;
            spdlog::info("Physical Device created");
            break;
        }
    }

    fatal(m_physical_device == VK_NULL_HANDLE, "Supported physical Device not found");
}

void VulkanDevice::destroy_physical_device() {
    if (m_physical_device != VK_NULL_HANDLE) {
        m_physical_device = VK_NULL_HANDLE;
        spdlog::info("Physical device destroyed");
    }
}

void VulkanDevice::create_logical_device(VkInstance instance) {
    check(instance != VK_NULL_HANDLE);
    check(m_physical_device != VK_NULL_HANDLE);

    std::unordered_set<uint32_t> unique_queue_families = {
        queue_indices.graphics_family,
        queue_indices.present_family
    };

    float queue_priority = 0.5f;

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .features = { .samplerAnisotropy = VK_TRUE },
    };

    VkPhysicalDeviceVulkan13Features vulkan13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = nullptr,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamic_state_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = nullptr,
    };

    features2.pNext = &vulkan13_features;
    vulkan13_features.pNext = &dynamic_state_features;

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features2,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(ms_device_extensions.size()),
        .ppEnabledExtensionNames = ms_device_extensions.data(),
    };

    vk_verify(vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device));

    vkGetDeviceQueue(m_device, queue_indices.graphics_family, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, queue_indices.present_family, 0, &m_present_queue);

    spdlog::info("Logical device created");
}

void VulkanDevice::destroy_logical_device() {
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);

        m_present_queue = VK_NULL_HANDLE;
        m_graphics_queue = VK_NULL_HANDLE;
        m_device = VK_NULL_HANDLE;

        queue_indices = QueueFamilyIndices{};

        spdlog::info("Logical device destroyed");
    }
}

bool VulkanDevice::is_physical_device_suitable(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    QueueFamilyIndices& queue_family_indices
) {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);

    if (!features.geometryShader) return false;
    if (!features.samplerAnisotropy) return false;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    if (properties.apiVersion < VK_API_VERSION_1_3) return false;

    if (!is_physical_device_supports_required_extensions(physical_device)) {
        return false;
    }

    uint32_t format_count = 0;
    vk_verify(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr));
    if (format_count == 0) return false;

    uint32_t present_mode_count = 0;
    vk_verify(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr));
    if (present_mode_count == 0) return false;

    const QueueFamilyIndices indices = find_queue_families(physical_device, surface);

    if (!indices.is_complete()) return false;

    queue_family_indices = indices;

    return true;
}

bool VulkanDevice::is_physical_device_supports_required_extensions(VkPhysicalDevice physical_device) {
    uint32_t extensions_count = 0;
    vk_verify(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, nullptr));

    std::vector<VkExtensionProperties> extensions(extensions_count);
    vk_verify(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, extensions.data()));

    for (const char* required : ms_device_extensions) {
        bool found = false;

        for (uint32_t i = 0; i < extensions_count; i++) {
            if (std::strcmp(required, extensions[i].extensionName) == 0) {
                found = true;
                spdlog::trace("Found required device extension: {}", required);
                break;
            }
        }

        if (!found) return false;
    }

    return true;
}

QueueFamilyIndices VulkanDevice::find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    check(physical_device != VK_NULL_HANDLE);
    check(surface != VK_NULL_HANDLE);

    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
            break;
        }
    }

    if (indices.graphics_family != UINT32_MAX) {
        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, indices.graphics_family, surface, &present_support);

        if (present_support) {
            indices.present_family = indices.graphics_family;
        }
    }

    if (indices.present_family == UINT32_MAX) {
        for (uint32_t i = 0; i < queue_family_count; i++) {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);

            if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support) {
                indices.graphics_family = i;
                indices.present_family = i;
                break;
            }
        }
    }

    if (indices.present_family == UINT32_MAX) {
        for (uint32_t i = 0; i < queue_family_count; i++) {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);

            if (present_support) {
                indices.present_family = i;
                break;
            }
        }
    }

    return indices;
}

} // namespace mantle