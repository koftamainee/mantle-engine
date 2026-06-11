// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <optional>
#include <vector>

#include "mantle/core/macros.h"
#include "mantle/core/types.h"
#include "mantle/keybinds/keybinds.h"

namespace mantle {
    class Window;
    class Engine;

    // clang-format off

#define MANTLE_ACTIONS(ns)                                                                         \
    namespace ns {                                                                                 \
        enum : mantle::InputSystem::Action {

#define MANTLE_ACTIONS_END()                                                                       \
            Count                                                                                  \
        };                                                                                         \
    }

    // clang-format on

    class InputSystem final {
      public:
        MANTLE_DEFAULT_INIT(InputSystem);

        using Action = u32;

        enum class DevicePreference : u8 {
            LastUsed,
            PreferController,
            PreferKeyboardMouse,
        };

        enum class ActiveDevice : u8 {
            KeyboardMouse,
            Controller,
        };

        struct KbmActionBinding {
            std::optional<Key>         key;
            std::optional<MouseButton> mouse_button;
            std::optional<Key>         axis_negative;
            std::optional<Key>         axis_positive;
            std::optional<MouseAxis>   mouse_axis;
            f32                        mouse_sensitivity = 1.0f;
            f32                        mouse_deadzone = 0.0f;
            // threshold for is_pressed / is_just_pressed on mouse axes
            // (in normalized [-1,1] space after sensitivity is applied)
            f32 mouse_threshold = 0.01f;
        };

        struct ControllerActionBinding {
            std::optional<ControllerButton> button;
            std::optional<ControllerAxis>   axis;
            f32                             deadzone = 0.15f;
            f32                             axis_threshold = 0.5f;
        };

        struct ActionBinding {
            KbmActionBinding        kbm {};
            ControllerActionBinding controller {};
        };

        void bind(Action action, Key key);
        void bind(Action action, MouseButton button);
        void bind(Action action, ControllerButton button);

        void bind_axis(Action action, Key negative, Key positive);
        void bind_axis(Action action, MouseAxis axis, f32 sensitivity = 1.0f, f32 deadzone = 0.0f);
        void bind_axis(Action action, ControllerAxis axis, f32 deadzone = 0.15f);

        void unbind(Action action);
        void clear_bindings();

        bool is_pressed(Action action) const;
        bool is_just_pressed(Action action) const;
        bool is_just_released(Action action) const;

        f32 get_axis(Action action) const;

        ActiveDevice get_active_device() const;
        bool         is_controller_active() const;
        void         set_device_preference(DevicePreference preference);

      private:
        friend class Engine;

        void init(Window *window, DevicePreference preference = DevicePreference::LastUsed);
        void destroy();
        void update();

        bool resolve_pressed(Action action) const;
        bool resolve_just_pressed(Action action) const;
        bool resolve_just_released(Action action) const;
        f32  resolve_axis(Action action) const;
        f32  resolve_mouse_axis_raw(const ActionBinding &b) const;

        ActiveDevice pick_active_device(const Window &window) const;

        bool is_action_valid(Action action) const;

        std::vector<ActionBinding> m_bindings;

        DevicePreference m_preference = DevicePreference::LastUsed;
        ActiveDevice     m_active_device = ActiveDevice::KeyboardMouse;

        f32 m_mouse_norm_x = 1.0f;
        f32 m_mouse_norm_y = 1.0f;

        Window *m_window;

        bool m_is_initialized = false;
    };

} // namespace mantle
