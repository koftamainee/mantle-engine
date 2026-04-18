#pragma once

#include <functional>
#include <string>

#include "GLFW/glfw3.h"
#include "core/memory/tlsf_allocator.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"
#include "glfw_allocator.h"

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
            Ctrl,
        };

        enum class MouseButton {
            Left,
            Middle,
            Right,
        };

      public:
        Window() = default;
        ~Window();

        MANTLE_NO_COPY_NO_MOVE(Window);

        void init(const Properties &properties, VirtualHeap *heap);
        void destroy();

        void update();
        bool should_close() const;

        bool is_key_pressed(Key key) const;
        bool is_key_just_pressed(Key key) const;
        bool is_key_just_released(Key key) const;

        bool is_mouse_button_pressed(MouseButton button) const;
        bool is_mouse_button_just_pressed(MouseButton button) const;
        bool is_mouse_button_just_released(MouseButton button) const;

        MousePosition get_mouse_position() const;


        u32 get_width() const;
        u32 get_height() const;
        Properties::Size get_size() const;
        Properties::Size get_framebuffer_size() const;
        GLFWwindow *get_native_window() const;
        f64 get_time() const;

        void set_resize_callback(std::function<void(u32, u32)> callback);

      private:
        static constexpr std::array<int, 7> glfw_keys = {
            GLFW_KEY_W,           GLFW_KEY_A,     GLFW_KEY_S,
            GLFW_KEY_D,           GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT,
            GLFW_KEY_LEFT_CONTROL};

        static constexpr std::array<int, 3> glfw_mb = {GLFW_MOUSE_BUTTON_LEFT,
                                                       GLFW_MOUSE_BUTTON_MIDDLE,
                                                       GLFW_MOUSE_BUTTON_RIGHT};


        bool m_is_initialized = false;
        GLFWwindow *m_native_window = nullptr;

        TlsfAllocator m_tlsf_alloc;
        GlfwAllocator m_glfw_alloc;

        std::array<bool, glfw_keys.size()> m_pressed_keys{};
        std::array<bool, glfw_keys.size()> m_pressed_keys_prev{};

        std::array<bool, glfw_mb.size()> m_pressed_mb{};
        std::array<bool, glfw_mb.size()> m_pressed_mb_prev{};

        std::function<void(u32, u32)> m_resize_callback{};
    };
} // namespace mantle
