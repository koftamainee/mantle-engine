#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan.h>

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

namespace mantle {
    class VulkanContext final {
      public:
        VulkanContext() = default;
        ~VulkanContext();

        VulkanContext(const VulkanContext &) = delete;
        VulkanContext &operator=(const VulkanContext &) = delete;
        VulkanContext(VulkanContext &&) noexcept = delete;
        VulkanContext &operator=(VulkanContext &&) noexcept = delete;

        void init(GLFWwindow *window);
        void destroy();

        VkInstance get_instance() const;
        VkSurfaceKHR get_surface() const;

      private:
        void create_instance();
        void destroy_instance();

#ifdef ENABLE_VALIDATION_LAYERS
        void create_debug_messenger_ext();
        void destroy_debug_messenger_ext();
#endif

        void create_surface(GLFWwindow *glfw_window);
        void destroy_surface();

      private:
        static std::vector<const char *> get_required_instance_extensions();

#ifdef ENABLE_VALIDATION_LAYERS
        static VkDebugUtilsMessengerCreateInfoEXT
        make_debug_messenger_create_info_ext();
        static void check_validation_layers();
#endif

      private:
        bool m_is_initialized = false;

#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
#endif

        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

      private:
#ifdef ENABLE_VALIDATION_LAYERS
        inline static const std::vector<const char *> ms_validation_layers{
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_MANGOHUD_overlay_x86_64",
        };
#endif
    };
} // namespace mantle
