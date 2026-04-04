#include "vulkan_graphics_context.h"

#include <core/assert.h>
#include <vkassert.h>
#include <cstring>
#include <iostream>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include "vkassert.h"

namespace {

#ifdef ENABLE_VALIDATION_LAYERS
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data) {

    constexpr auto type_to_str =
        [](const VkDebugUtilsMessageTypeFlagsEXT t) -> const char* {
        if (t & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            return "validation";
        if (t & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            return "performance";
        if (t & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
            return "device_address";
        if (t & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
            return "general";
        return "unknown";
    };

    auto msg = fmt::format("[vulkan] [{}] {}", type_to_str(type), p_callback_data->pMessage);

    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        spdlog::trace(msg);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        spdlog::info(msg);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn(msg);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error(msg);
        break;
    default:
        spdlog::info(msg);
        break;
    }

    (void)p_user_data;
    return VK_FALSE;
}
#endif

} // namespace

namespace mantle {

VulkanGraphicsContext::~VulkanGraphicsContext() {
    destroy();
}

void VulkanGraphicsContext::init(GLFWwindow* window) {
    check(!m_is_initialized);

    create_instance();

#ifdef ENABLE_VALIDATION_LAYERS
    create_debug_messenger_ext();
#endif

    create_surface(window);
    m_is_initialized = true;
}

void VulkanGraphicsContext::destroy() {
    if (m_is_initialized) {
        destroy_surface();

#ifdef ENABLE_VALIDATION_LAYERS
        destroy_debug_messenger_ext();
#endif

        destroy_instance();
        m_is_initialized = false;
    }
}

VkInstance VulkanGraphicsContext::get_instance() const {
    return m_instance;
}

VkSurfaceKHR VulkanGraphicsContext::get_surface() const {
    return m_surface;
}

void VulkanGraphicsContext::create_instance() {
    constexpr VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "VkEngine",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "VkEngine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    const std::vector<const char*> extensions = get_required_instance_extensions();

#ifndef ENABLE_VALIDATION_LAYERS
    const
#endif
    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

#ifdef ENABLE_VALIDATION_LAYERS
    const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info =
        make_debug_messenger_create_info_ext();

    instance_create_info.pNext = &debug_messenger_create_info;
    instance_create_info.enabledLayerCount =
        static_cast<uint32_t>(ms_validation_layers.size());
    instance_create_info.ppEnabledLayerNames = ms_validation_layers.data();
#endif

    vk_verify(vkCreateInstance(&instance_create_info, nullptr, &m_instance));
    spdlog::info("Vulkan Instance Created");
}

void VulkanGraphicsContext::destroy_instance() {
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
        spdlog::info("Vulkan Instance Destroyed");
    }
}

#ifdef ENABLE_VALIDATION_LAYERS
void VulkanGraphicsContext::create_debug_messenger_ext() {
    check(m_instance != VK_NULL_HANDLE);

    const auto vk_create_debug_utils_messenger =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));

    check(vk_create_debug_utils_messenger != nullptr);

    const VkDebugUtilsMessengerCreateInfoEXT create_info =
        make_debug_messenger_create_info_ext();

    check_validation_layers();

    vk_verify(vk_create_debug_utils_messenger(m_instance, &create_info, nullptr, &m_debug_messenger));
    spdlog::info("Debug Messenger Created");
}

void VulkanGraphicsContext::destroy_debug_messenger_ext() {
    if (m_debug_messenger != VK_NULL_HANDLE) {
        check(m_instance != VK_NULL_HANDLE);

        const auto vk_destroy_debug_utils_messenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (vk_destroy_debug_utils_messenger != nullptr) {
            vk_destroy_debug_utils_messenger(m_instance, m_debug_messenger, nullptr);
        }

        m_debug_messenger = VK_NULL_HANDLE;
        spdlog::info("Debug Messenger destroyed");
    }
}
#endif

void VulkanGraphicsContext::create_surface(GLFWwindow* glfw_window) {
    check(m_instance != VK_NULL_HANDLE);

    fatal(glfwCreateWindowSurface(m_instance, glfw_window, nullptr, &m_surface) != VK_SUCCESS,
          "Failed to create surface");

    spdlog::info("Surface Created");
}

void VulkanGraphicsContext::destroy_surface() {
    if (m_surface != VK_NULL_HANDLE) {
        check(m_instance != VK_NULL_HANDLE);

        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;

        spdlog::info("Surface destroyed");
    }
}

std::vector<const char*> VulkanGraphicsContext::get_required_instance_extensions() {
    uint32_t glfw_extensions_count = 0;
    const char** glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    fatal(glfw_extensions_count == 0, "Failed to get GLFW extensions");

    uint32_t vk_extensions_count = 0;
    vk_verify(vkEnumerateInstanceExtensionProperties(nullptr, &vk_extensions_count, nullptr));

    std::vector<VkExtensionProperties> vk_extensions(vk_extensions_count);
    vk_verify(vkEnumerateInstanceExtensionProperties(nullptr, &vk_extensions_count, vk_extensions.data()));

    for (uint32_t i = 0; i < glfw_extensions_count; i++) {
        bool found = false;
        for (const auto& [extension_name, _] : vk_extensions) {
            if (std::strcmp(extension_name, glfw_extensions[i]) == 0) {
                found = true;
                spdlog::trace("Found extension: {}", extension_name);
                break;
            }
        }
        fatal(!found, "Required GLFW extensions are not supported");
    }

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extensions_count);

#ifdef ENABLE_VALIDATION_LAYERS
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    spdlog::info("Validation layers are enabled. Enabling VK_EXT_debug_utils");
#endif

    return extensions;
}

#ifdef ENABLE_VALIDATION_LAYERS

VkDebugUtilsMessengerCreateInfoEXT VulkanGraphicsContext::make_debug_messenger_create_info_ext() {
    return {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &debug_callback,
        .pUserData = nullptr,
    };
}

void VulkanGraphicsContext::check_validation_layers() {
    uint32_t vk_layer_properties_count = 0;
    vk_verify(vkEnumerateInstanceLayerProperties(&vk_layer_properties_count, nullptr));

    std::vector<VkLayerProperties> vk_layers(vk_layer_properties_count);
    vk_verify(vkEnumerateInstanceLayerProperties(&vk_layer_properties_count, vk_layers.data()));

    for (auto const needed_layer : ms_validation_layers) {
        bool found = false;
        for (const auto& vk_layer : vk_layers) {
            if (strcmp(needed_layer, vk_layer.layerName) == 0) {
                found = true;
                spdlog::trace("Found validation layer: {}", needed_layer);
                break;
            }
        }
        fatal(!found, "Required VK_LAYERS are not supported");
    }
}

#endif

} // namespace mantle
