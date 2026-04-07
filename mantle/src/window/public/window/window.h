#pragma once

#include <functional>
#include <string>
#include "GLFW/glfw3.h"
#include "core/types.h"

namespace mantle {
    class Window final {
      public:
        struct Properties {
            struct Size {
                u32 width;
                u32 height;
            };

            std::string title;
            Size size;
        };
        struct MousePosition final {
            f32 x;
            f32 y;
        };
        enum class Key {
            W,
            A,
            S,
            D,
            Space,
            Shift,
        };

        enum class MouseButton {
            Left, Middle, Right,
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

        void update() const;
        bool should_close() const;

        bool is_key_pressed(Key key) const;
        bool is_mouse_button_pressed(MouseButton mouse_button) const;
        MousePosition get_mouse_position() const;


        u32 get_width() const;
        u32 get_height() const;
        Properties::Size get_size() const;
        Properties::Size get_framebuffer_size() const;
        GLFWwindow *get_native_window() const;
        f64 get_time() const;

        void set_resize_callback(std::function<void(u32, u32)> callback);

      private:
        bool m_is_initialized = false;
        GLFWwindow *m_native_window = nullptr;

        std::function<void(u32, u32)> m_resize_callback{};

        inline static u32 s_windows_count = 0;
    };
} // namespace mantle
