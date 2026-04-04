#pragma once

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
        GLFWwindow *get_native_window() const;

    private:
        bool m_is_initialized = false;
        GLFWwindow *m_native_window = nullptr;

        inline static uint32_t s_windows_count = 0;
    };
}
