// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <bitset>
#include <functional>
#include <span>

#include "mantle/core/macros.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/types.h"
#include "mantle/keybinds/keybinds.h"

namespace spdlog {
    class logger;
}

namespace mantle {
    class Window final {
      public:
        struct Properties {
            struct Size {
                u32 width;
                u32 height;
            };

            std::string_view title = "Mantle";
            Size             size = {0, 0};
            bool             fullscreen = true;
        };

        struct RawEvent {
            enum class Type : u8 {
                Key,
                MouseButton,
                MouseMotion,
                MouseWheel,
                ControllerButton,
                ControllerAxis,
            };

            Type type;
            union {
                struct {
                    Key  key;
                    bool pressed;
                } key;
                struct {
                    MouseButton button;
                    bool        pressed;
                } mouse_button;
                struct {
                    f32 xrel, yrel;
                } mouse_motion;
                struct {
                    f32 x, y;
                } mouse_wheel;
                struct {
                    ControllerButton button;
                    bool             pressed;
                } controller_button;
                struct {
                    ControllerAxis axis;
                    f32            value;
                } controller_axis;
            };
        };

      public:
        MANTLE_DEFAULT_INIT(Window);

        void init(const Properties &properties, MemoryBlock mem);
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

        bool is_keyboard_active_this_frame() const;
        bool is_mouse_active_this_frame() const;
        bool is_controller_active_this_frame() const;

        bool is_controller_connected() const;
        bool is_controller_button_pressed(ControllerButton button) const;
        bool is_controller_button_just_pressed(ControllerButton button) const;
        bool is_controller_button_just_released(ControllerButton button) const;
        f32  get_controller_axis(ControllerAxis axis) const;

        void set_controller_rumble(u16 low_frequency, u16 high_frequency, u32 duration_ms);
        void stop_controller_rumble();

        u32                       get_width() const;
        u32                       get_height() const;
        Properties::Size          get_size() const;
        Properties::Size          get_framebuffer_size() const;
        SDL_Window               *get_native_window() const;
        u64                       get_time_ns() const;
        u64                       get_time_ms() const;
        bool                      is_fullscreen() const;
        f32                       get_refresh_rate() const;
        std::span<const RawEvent> get_frame_events() const;
        MouseDelta                get_mouse_delta() const;
        MouseDelta                get_mouse_wheel() const;

        void set_resize_callback(std::function<void(u32, u32)> callback);

      private:
        static constexpr usize MOUSE_BUTTON_COUNT = 8;
        static constexpr usize CONTROLLER_BUTTON_COUNT = 15;
        static constexpr usize CONTROLLER_AXIS_COUNT = 6;

        bool m_is_initialized = false;
        bool m_should_close = false;

        SDL_Window  *m_native_window = nullptr;
        SDL_Gamepad *m_controller = nullptr;

        std::bitset<static_cast<usize>(Key::Count)> m_pressed_keys;
        std::bitset<static_cast<usize>(Key::Count)> m_pressed_keys_prev;

        std::array<bool, MOUSE_BUTTON_COUNT> m_pressed_mb {};
        std::array<bool, MOUSE_BUTTON_COUNT> m_pressed_mb_prev {};

        std::array<bool, CONTROLLER_BUTTON_COUNT> m_pressed_cb {};
        std::array<bool, CONTROLLER_BUTTON_COUNT> m_pressed_cb_prev {};

        std::array<f32, CONTROLLER_AXIS_COUNT> m_controller_axes {};

        static constexpr usize                     MAX_EVENTS_PER_FRAME = 128;
        std::array<RawEvent, MAX_EVENTS_PER_FRAME> m_frame_events {};
        u32                                        m_frame_event_count = 0;

        MouseDelta m_mouse_delta {};
        MouseDelta m_mouse_wheel {};

        bool m_keyboard_active = false;
        bool m_mouse_active = false;
        bool m_controller_active = false;

        std::function<void(u32, u32)> m_resize_callback {};

        bool m_fullscreen = false;
        f32  m_refresh_rate = 0.0f;

        spdlog::logger *m_logger = nullptr;
    };
} // namespace mantle
