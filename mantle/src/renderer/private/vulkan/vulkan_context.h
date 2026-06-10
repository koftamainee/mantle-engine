// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <array>
#include <vector>

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "core/memory/arena_allocator.h"

namespace spdlog {
    class logger;
}

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

namespace mantle {
    class VulkanContext final {
      public:
        MANTLE_DEFAULT_INIT(VulkanContext);

        void init(SDL_Window *window, ArenaAllocator *scratch_arena,
                  VkAllocationCallbacks *vk_callbacks);
        void destroy();

        VkInstance   get_instance() const;
        VkSurfaceKHR get_surface() const;

      private:
#ifdef ENABLE_VALIDATION_LAYERS
#ifdef _WIN32
        static constexpr std::array<const char *, 1> ms_validation_layers {
            "VK_LAYER_KHRONOS_validation"};
#elif defined(__linux__)
        static constexpr std::array<const char *, 2> ms_validation_layers {
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_MANGOHUD_overlay_x86_64",
        };
#elif defined(__APPLE__)
#undef ENABLE_VALIDATION_LAYERS
#endif
#endif

        void create_instance();
        void destroy_instance();

#ifdef ENABLE_VALIDATION_LAYERS
        void create_debug_messenger_ext();
        void destroy_debug_messenger_ext();
#endif

        void create_surface(SDL_Window *window);
        void destroy_surface();

      private:
        static std::pmr::vector<const char *>
        get_required_instance_extensions(std::pmr::memory_resource &resource);

#ifdef ENABLE_VALIDATION_LAYERS
        static VkDebugUtilsMessengerCreateInfoEXT make_debug_messenger_create_info_ext();
        static void check_validation_layers(std::pmr::memory_resource &memory);
#endif

      private:
        bool m_is_initialized = false;

        ArenaAllocator        *m_scratch_arena = nullptr;
        VkAllocationCallbacks *m_alloc_callbacks = nullptr;

#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
#endif

        VkInstance   m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

        spdlog::logger *m_logger = nullptr;
    };
} // namespace mantle
