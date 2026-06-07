#include "vulkan/vulkan_context.h"

#include <core/assert.h>
#include "vkassert.h"

#include <spdlog/spdlog.h>

#include "core/memory/pmr/arena_resource.h"
#include "core/memory/scope_arena.h"

namespace {

#ifdef ENABLE_VALIDATION_LAYERS
    VKAPI_ATTR VkBool32 VKAPI_CALL
    debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                   VkDebugUtilsMessageTypeFlagsEXT type,
                   const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
                   void *p_user_data) {

        constexpr auto type_to_str =
            [](const VkDebugUtilsMessageTypeFlagsEXT t) -> const char * {
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

        auto msg = fmt::format("[VVL] [{}] {}", type_to_str(type),
                               p_callback_data->pMessage);

        switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            spdlog::get("vulkan")->trace(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            spdlog::get("vulkan")->info(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            spdlog::get("vulkan")->warn(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            spdlog::get("vulkan")->error(msg);
            break;
        default:
            spdlog::get("vulkan")->info(msg);
            break;
        }

        (void)p_user_data;
        return VK_FALSE;
    }
#endif

} // namespace

namespace mantle {

    VulkanContext::~VulkanContext() { destroy(); }

    void VulkanContext::init(SDL_Window *window, ArenaAllocator *scratch_arena,
                             VkAllocationCallbacks *vk_callbacks) {
        MANTLE_CHECK(!m_is_initialized);

        m_logger = spdlog::get("vulkan").get();
        m_alloc_callbacks = vk_callbacks;
        m_scratch_arena = scratch_arena;

        create_instance();

#ifdef ENABLE_VALIDATION_LAYERS
        create_debug_messenger_ext();
#endif

        create_surface(window);
        m_is_initialized = true;
    }

    void VulkanContext::destroy() {
        if (m_is_initialized) {
            destroy_surface();

#ifdef ENABLE_VALIDATION_LAYERS
            destroy_debug_messenger_ext();
#endif

            destroy_instance();
            m_alloc_callbacks = nullptr;
            m_is_initialized = false;
        }
    }

    VkInstance VulkanContext::get_instance() const { return m_instance; }

    VkSurfaceKHR VulkanContext::get_surface() const { return m_surface; }

    void VulkanContext::create_instance() {
        constexpr VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Mantle",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Mantle engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3,
        };

        ScopeArena scope(m_scratch_arena);
        ArenaResource resource(m_scratch_arena);

        const std::pmr::vector<const char *> extensions =
            get_required_instance_extensions(resource);

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
                .enabledExtensionCount = static_cast<u32>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
            };

#ifdef ENABLE_VALIDATION_LAYERS
        const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info =
            make_debug_messenger_create_info_ext();

        instance_create_info.pNext = &debug_messenger_create_info;
        instance_create_info.enabledLayerCount =
            static_cast<u32>(ms_validation_layers.size());
        instance_create_info.ppEnabledLayerNames = ms_validation_layers.data();
#endif

        MANTLE_VK_VERIFY(vkCreateInstance(&instance_create_info, m_alloc_callbacks,
                                   &m_instance));
        m_logger->info("Vulkan instance created");
    }

    void VulkanContext::destroy_instance() {
        if (m_instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_instance, m_alloc_callbacks);
            m_instance = VK_NULL_HANDLE;
            m_logger->info("Vulkan instance destroyed");
        }
    }

#ifdef ENABLE_VALIDATION_LAYERS
    void VulkanContext::create_debug_messenger_ext() {
        MANTLE_CHECK(m_instance != VK_NULL_HANDLE);

        const auto vk_create_debug_utils_messenger =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_instance,
                                      "vkCreateDebugUtilsMessengerEXT"));

        MANTLE_CHECK(vk_create_debug_utils_messenger != nullptr);

        const VkDebugUtilsMessengerCreateInfoEXT create_info =
            make_debug_messenger_create_info_ext();

        ScopeArena scope(m_scratch_arena);
        ArenaResource resource(m_scratch_arena);
        check_validation_layers(resource);


        MANTLE_VK_VERIFY(vk_create_debug_utils_messenger(
            m_instance, &create_info, m_alloc_callbacks, &m_debug_messenger));
        m_logger->info("Debug messenger created");
    }

    void VulkanContext::destroy_debug_messenger_ext() {
        if (m_debug_messenger != VK_NULL_HANDLE) {
            MANTLE_CHECK(m_instance != VK_NULL_HANDLE);

            const auto vk_destroy_debug_utils_messenger =
                reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(m_instance,
                                          "vkDestroyDebugUtilsMessengerEXT"));

            if (vk_destroy_debug_utils_messenger != nullptr) {
                vk_destroy_debug_utils_messenger(m_instance, m_debug_messenger,
                                                 m_alloc_callbacks);
            }

            m_debug_messenger = VK_NULL_HANDLE;
            m_logger->info("Debug messenger destroyed");
        }
    }
#endif

    void VulkanContext::create_surface(SDL_Window *window) {
        MANTLE_CHECK(m_instance != VK_NULL_HANDLE);

        MANTLE_FATAL(!SDL_Vulkan_CreateSurface(window, m_instance,
                                        m_alloc_callbacks, &m_surface),
              "Failed to create surface");

        m_logger->info("Surface created");
    }

    void VulkanContext::destroy_surface() {
        if (m_surface != VK_NULL_HANDLE) {
            MANTLE_CHECK(m_instance != VK_NULL_HANDLE);

            vkDestroySurfaceKHR(m_instance, m_surface, m_alloc_callbacks);
            m_surface = VK_NULL_HANDLE;

            m_logger->info("Surface destroyed");
        }
    }

    std::pmr::vector<const char *>
    VulkanContext::get_required_instance_extensions(
        std::pmr::memory_resource &resource) {
        Uint32 sdl_extensions_count = 0;
        const char *const *sdl_extensions =
            SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
        MANTLE_FATAL(sdl_extensions == nullptr,
              "Failed to get SDL Vulkan extensions");

        u32 vk_extensions_count = 0;
        MANTLE_VK_VERIFY(vkEnumerateInstanceExtensionProperties(
            nullptr, &vk_extensions_count, nullptr));

        std::pmr::vector<VkExtensionProperties> vk_extensions(&resource);
        vk_extensions.resize(vk_extensions_count);
        MANTLE_VK_VERIFY(vkEnumerateInstanceExtensionProperties(
            nullptr, &vk_extensions_count, vk_extensions.data()));

        for (Uint32 i = 0; i < sdl_extensions_count; i++) {
            bool found = false;
            for (const auto &[extension_name, _] : vk_extensions) {
                if (std::strcmp(extension_name, sdl_extensions[i]) == 0) {
                    found = true;
                    spdlog::get("vulkan")->trace("Found extension: {}",
                                                 extension_name);
                    break;
                }
            }
            MANTLE_FATAL(!found, "Required SDL Vulkan extensions are not supported");
        }

        std::pmr::vector<const char *> extensions(&resource);
#ifdef ENABLE_VALIDATION_LAYERS
        extensions.resize(sdl_extensions_count + 1);
#else
        extensions.resize(sdl_extensions_count);
#endif

        for (Uint32 i = 0; i < sdl_extensions_count; i++) {
            extensions[i] = sdl_extensions[i];
        }

#ifdef ENABLE_VALIDATION_LAYERS
        extensions[sdl_extensions_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        spdlog::get("vulkan")->info(
            "Validation layers are enabled. Enabling VK_EXT_debug_utils");
#endif

        return extensions;
    }

#ifdef ENABLE_VALIDATION_LAYERS

    VkDebugUtilsMessengerCreateInfoEXT
    VulkanContext::make_debug_messenger_create_info_ext() {
        return {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = &debug_callback,
            .pUserData = nullptr,
        };
    }

    void
    VulkanContext::check_validation_layers(std::pmr::memory_resource &memory) {
        u32 vk_layer_properties_count = 0;
        MANTLE_VK_VERIFY(vkEnumerateInstanceLayerProperties(&vk_layer_properties_count,
                                                     nullptr));

        std::pmr::vector<VkLayerProperties> vk_layers(&memory);
        vk_layers.resize(vk_layer_properties_count);
        MANTLE_VK_VERIFY(vkEnumerateInstanceLayerProperties(&vk_layer_properties_count,
                                                     vk_layers.data()));

        for (auto const needed_layer : ms_validation_layers) {
            bool found = false;
            for (const auto &vk_layer : vk_layers) {
                if (strcmp(needed_layer, vk_layer.layerName) == 0) {
                    found = true;
                    spdlog::get("vulkan")->trace(
                        "Found validation layer: {}", needed_layer);
                    break;
                }
            }
            MANTLE_FATAL(!found, "Required VK_LAYERS are not supported");
        }
    }

#endif

} // namespace mantle
