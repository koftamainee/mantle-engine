#pragma once

#include <array>
#include <bitset>
#include <functional>
#include <span>
#include <string>

#include <SDL3/SDL.h>

#include "core/macros.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"

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
            Size size = {0, 0};
            bool fullscreen = true;
        };
        struct MousePosition final {
            f32 x;
            f32 y;
        };
        struct MouseDelta final {
            f32 x;
            f32 y;
        };
        enum class Key : u16 {
            Unknown = 0,
            A = 4,
            B = 5,
            C = 6,
            D = 7,
            E = 8,
            F = 9,
            G = 10,
            H = 11,
            I = 12,
            J = 13,
            K = 14,
            L = 15,
            M = 16,
            N = 17,
            O = 18,
            P = 19,
            Q = 20,
            R = 21,
            S = 22,
            T = 23,
            U = 24,
            V = 25,
            W = 26,
            X = 27,
            Y = 28,
            Z = 29,
            _1 = 30,
            _2 = 31,
            _3 = 32,
            _4 = 33,
            _5 = 34,
            _6 = 35,
            _7 = 36,
            _8 = 37,
            _9 = 38,
            _0 = 39,
            Return = 40,
            Escape = 41,
            Backspace = 42,
            Tab = 43,
            Space = 44,
            Minus = 45,
            Equals = 46,
            LBracket = 47,
            RBracket = 48,
            Backslash = 49,
            NonUsHash = 50,
            Semicolon = 51,
            Apostrophe = 52,
            Grave = 53,
            Comma = 54,
            Period = 55,
            Slash = 56,
            CapsLock = 57,
            F1 = 58,
            F2 = 59,
            F3 = 60,
            F4 = 61,
            F5 = 62,
            F6 = 63,
            F7 = 64,
            F8 = 65,
            F9 = 66,
            F10 = 67,
            F11 = 68,
            F12 = 69,
            PrintScreen = 70,
            ScrollLock = 71,
            Pause = 72,
            Insert = 73,
            Home = 74,
            PageUp = 75,
            Delete = 76,
            End = 77,
            PageDown = 78,
            Right = 79,
            Left = 80,
            Down = 81,
            Up = 82,
            NumLock = 83,
            KPDivide = 84,
            KPMultiply = 85,
            KPMinus = 86,
            KPPlus = 87,
            KPEnter = 88,
            KP1 = 89,
            KP2 = 90,
            KP3 = 91,
            KP4 = 92,
            KP5 = 93,
            KP6 = 94,
            KP7 = 95,
            KP8 = 96,
            KP9 = 97,
            KP0 = 98,
            KPPeriod = 99,
            NonUsBackslash = 100,
            Application = 101,
            Power = 102,
            KPEquals = 103,
            F13 = 104,
            F14 = 105,
            F15 = 106,
            F16 = 107,
            F17 = 108,
            F18 = 109,
            F19 = 110,
            F20 = 111,
            F21 = 112,
            F22 = 113,
            F23 = 114,
            F24 = 115,
            Execute = 116,
            Help = 117,
            Menu = 118,
            Select = 119,
            Stop = 120,
            Again = 121,
            Undo = 122,
            Cut = 123,
            Copy = 124,
            Paste = 125,
            Find = 126,
            Mute = 127,
            VolumeUp = 128,
            VolumeDown = 129,
            AudioNext = 130,
            AudioPrev = 131,
            AudioStop = 132,
            AudioPlay = 133,
            AudioMute = 134,
            MediaSelect = 135,
            WWW = 136,
            Mail = 137,
            Calculator = 138,
            Computer = 139,
            ACSearch = 140,
            ACHome = 141,
            ACBack = 142,
            ACForward = 143,
            ACStop = 144,
            ACRefresh = 145,
            ACBookmarks = 146,
            BrightnessDown = 147,
            BrightnessUp = 148,
            DisplaySwitch = 149,
            KbdIllumToggle = 150,
            KbdIllumDown = 151,
            KbdIllumUp = 152,
            Eject = 153,
            Sleep = 154,
            App1 = 155,
            App2 = 156,
            AudioRewind = 157,
            AudioFastForward = 158,
            LControl = 224,
            LShift = 225,
            LAlt = 226,
            LGUI = 227,
            RControl = 228,
            RShift = 229,
            RAlt = 230,
            RGUI = 231,
            Count = 512,
        };

        enum class MouseButton : u8 {
            Left = 1,
            Middle = 2,
            Right = 3,
            X1 = 4,
            X2 = 5,
        };

        enum class ControllerButton : u8 {
            A = 0,
            B = 1,
            X = 2,
            Y = 3,
            Back = 4,
            Guide = 5,
            Start = 6,
            LeftStick = 7,
            RightStick = 8,
            LeftShoulder = 9,
            RightShoulder = 10,
            DPadUp = 11,
            DPadDown = 12,
            DPadLeft = 13,
            DPadRight = 14,
        };

        enum class ControllerAxis : u8 {
            LeftX = 0,
            LeftY = 1,
            RightX = 2,
            RightY = 3,
            LeftTrigger = 4,
            RightTrigger = 5,
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
                    Key key;
                    bool pressed;
                } key;
                struct {
                    MouseButton button;
                    bool pressed;
                } mouse_button;
                struct {
                    f32 xrel, yrel;
                } mouse_motion;
                struct {
                    f32 x, y;
                } mouse_wheel;
                struct {
                    ControllerButton button;
                    bool pressed;
                } controller_button;
                struct {
                    ControllerAxis axis;
                    f32 value;
                } controller_axis;
            };
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

        bool is_keyboard_active() const;
        bool is_mouse_active() const;
        bool is_controller_active() const;

        bool is_controller_connected() const;
        bool is_controller_button_pressed(ControllerButton button) const;
        f32 get_controller_axis(ControllerAxis axis) const;

        void set_controller_rumble(u16 low_frequency, u16 high_frequency,
                                   u32 duration_ms);
        void stop_controller_rumble();

        u32 get_width() const;
        u32 get_height() const;
        Properties::Size get_size() const;
        Properties::Size get_framebuffer_size() const;
        SDL_Window *get_native_window() const;
        u64 get_time_ns() const;
        u64 get_time_ms() const;
        bool is_fullscreen() const;
        f32 get_refresh_rate() const;
        std::span<const RawEvent> get_frame_events() const;
        MouseDelta get_mouse_delta() const;
        MouseDelta get_mouse_wheel() const;

        void set_resize_callback(std::function<void(u32, u32)> callback);

      private:
        static constexpr usize MOUSE_BUTTON_COUNT = 8;
        static constexpr usize CONTROLLER_BUTTON_COUNT = 15;
        static constexpr usize CONTROLLER_AXIS_COUNT = 6;

        bool m_is_initialized = false;
        bool m_should_close = false;

        SDL_Window *m_native_window = nullptr;
        SDL_Gamepad *m_controller = nullptr;

        std::bitset<static_cast<usize>(Key::Count)> m_pressed_keys;
        std::bitset<static_cast<usize>(Key::Count)> m_pressed_keys_prev;

        std::array<bool, MOUSE_BUTTON_COUNT> m_pressed_mb{};
        std::array<bool, MOUSE_BUTTON_COUNT> m_pressed_mb_prev{};

        std::array<bool, CONTROLLER_BUTTON_COUNT> m_pressed_cb{};
        std::array<bool, CONTROLLER_BUTTON_COUNT> m_pressed_cb_prev{};

        std::array<f32, CONTROLLER_AXIS_COUNT> m_controller_axes{};

        static constexpr usize MAX_EVENTS_PER_FRAME = 128;
        std::array<RawEvent, MAX_EVENTS_PER_FRAME> m_frame_events{};
        u32 m_frame_event_count = 0;

        MouseDelta m_mouse_delta{};
        MouseDelta m_mouse_wheel{};

        bool m_keyboard_active = false;
        bool m_mouse_active = false;
        bool m_controller_active = false;

        std::function<void(u32, u32)> m_resize_callback{};

        bool m_fullscreen = false;
        f32 m_refresh_rate = 0.0f;

        spdlog::logger *m_logger = nullptr;
    };
} // namespace mantle
