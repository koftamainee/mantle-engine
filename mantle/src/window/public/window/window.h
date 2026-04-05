#pragma once

#include <functional>
#include <string>
#include "GLFW/glfw3.h"

namespace mantle {
    class Window final {
    public:
        struct Properties {
            struct Size {
                uint32_t width;
                uint32_t height;
            };

            std::string title;
            Size size;
        };

    public:
        Window() = default;
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;
        Window(Window &&) noexcept = delete;
        Window &operator=(Window &&) noexcept = delete;

        void init(const Properties &properties);
        void destroy();

        void on_update() const;
        bool should_close() const;


        uint32_t get_width() const;
        uint32_t get_height() const;
        Properties::Size get_size() const;
        Properties::Size get_framebuffer_size() const;
        GLFWwindow *get_native_window() const;
        double get_time() const;

        void set_resize_callback(std::function<void(uint32_t, uint32_t)> callback);

    private:
        bool m_is_initialized = false;
        GLFWwindow *m_native_window = nullptr;

        std::function<void(uint32_t, uint32_t)> m_resize_callback{};

        inline static uint32_t s_windows_count = 0;
    };
}
