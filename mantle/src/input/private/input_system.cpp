// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/input/input_system.h"

#include <cmath>

#include "mantle/window/window.h"

namespace mantle {

    void InputSystem::init(Window *window, DevicePreference preference) {
        MANTLE_CHECK(!m_is_initialized);
        m_window = window;
        m_preference = preference;
        m_is_initialized = true;
    }

    void InputSystem::destroy() {
        if (m_is_initialized) {
            m_bindings.clear();
            m_is_initialized = false;
        }
    }

    void InputSystem::update() {
        MANTLE_CHECK(m_is_initialized);
        m_active_device = pick_active_device(*m_window);

        const auto size = m_window->get_size();
        m_mouse_norm_x = size.width > 0 ? 2.0f / static_cast<f32>(size.width) : 1.0f;
        m_mouse_norm_y = size.height > 0 ? 2.0f / static_cast<f32>(size.height) : 1.0f;
    }

    InputSystem::ActiveDevice InputSystem::pick_active_device(const Window &window) const {
        MANTLE_CHECK(m_is_initialized);
        switch (m_preference) {
            case DevicePreference::PreferController:
                return window.is_controller_connected() ? ActiveDevice::Controller
                                                        : ActiveDevice::KeyboardMouse;
            case DevicePreference::PreferKeyboardMouse:
                return ActiveDevice::KeyboardMouse;
            case DevicePreference::LastUsed:
            default:
                if (window.is_controller_active_this_frame()) {
                    return ActiveDevice::Controller;
                }
                if (window.is_keyboard_active_this_frame()) {
                    return ActiveDevice::KeyboardMouse;
                }
                if (window.is_mouse_active_this_frame()) {
                    return ActiveDevice::KeyboardMouse;
                }
                return m_active_device;
        }
    }

    bool InputSystem::is_action_valid(Action action) const {
        return action < static_cast<Action>(m_bindings.size());
    }

    void InputSystem::bind(Action action, Key key) {
        if (action >= static_cast<Action>(m_bindings.size())) {
            m_bindings.resize(action + 1);
        }
        m_bindings[action].kbm.key = key;
    }

    void InputSystem::bind(Action action, MouseButton button) {
        if (action >= static_cast<Action>(m_bindings.size())) {
            m_bindings.resize(action + 1);
        }
        m_bindings[action].kbm.mouse_button = button;
    }

    void InputSystem::bind(Action action, ControllerButton button) {
        if (action >= static_cast<Action>(m_bindings.size())) {
            m_bindings.resize(action + 1);
        }
        m_bindings[action].controller.button = button;
    }

    void InputSystem::bind_axis(Action action, Key negative, Key positive) {
        if (action >= static_cast<Action>(m_bindings.size())) {
            m_bindings.resize(action + 1);
        }
        m_bindings[action].kbm.axis_negative = negative;
        m_bindings[action].kbm.axis_positive = positive;
    }

    void InputSystem::bind_axis(Action action, MouseAxis axis, f32 sensitivity, f32 deadzone) {
        if (action >= static_cast<Action>(m_bindings.size())) {
            m_bindings.resize(action + 1);
        }
        m_bindings[action].kbm.mouse_axis = axis;
        m_bindings[action].kbm.mouse_sensitivity = sensitivity;
        m_bindings[action].kbm.mouse_deadzone = deadzone;
    }

    void InputSystem::bind_axis(Action action, ControllerAxis axis, f32 deadzone) {
        if (action >= static_cast<Action>(m_bindings.size())) {
            m_bindings.resize(action + 1);
        }
        m_bindings[action].controller.axis = axis;
        m_bindings[action].controller.deadzone = deadzone;
    }

    void InputSystem::unbind(Action action) {
        MANTLE_CHECK(is_action_valid(action));
        if (action >= static_cast<Action>(m_bindings.size())) {
            m_bindings.resize(action + 1);
        }
        m_bindings[action] = {};
    }

    void InputSystem::clear_bindings() {
        for (auto &b : m_bindings) {
            b = {};
        }
    }

    bool InputSystem::is_pressed(Action action) const {
        MANTLE_CHECK(is_action_valid(action));
        return resolve_pressed(action);
    }

    bool InputSystem::is_just_pressed(Action action) const {
        MANTLE_CHECK(is_action_valid(action));
        return resolve_just_pressed(action);
    }

    bool InputSystem::is_just_released(Action action) const {
        MANTLE_CHECK(is_action_valid(action));
        return resolve_just_released(action);
    }

    f32 InputSystem::get_axis(Action action) const {
        MANTLE_CHECK(is_action_valid(action));
        return resolve_axis(action);
    }

    InputSystem::ActiveDevice InputSystem::get_active_device() const { return m_active_device; }

    bool InputSystem::is_controller_active() const {
        return m_active_device == ActiveDevice::Controller;
    }

    void InputSystem::set_device_preference(DevicePreference preference) {
        m_preference = preference;
    }

    // Returns the normalized raw value for a mouse axis binding (sensitivity NOT applied).
    f32 InputSystem::resolve_mouse_axis_raw(const ActionBinding &b) const {
        const auto *window = m_window;
        f32         raw = 0.0f;
        switch (*b.kbm.mouse_axis) {
            case MouseAxis::MoveX:
                raw = window->get_mouse_delta().x * m_mouse_norm_x;
                break;
            case MouseAxis::MoveY:
                raw = window->get_mouse_delta().y * m_mouse_norm_y;
                break;
            case MouseAxis::WheelX:
                raw = window->get_mouse_wheel().x;
                break;
            case MouseAxis::WheelY:
                raw = window->get_mouse_wheel().y;
                break;
        }
        return std::abs(raw) < b.kbm.mouse_deadzone ? 0.0f : raw;
    }

    bool InputSystem::resolve_pressed(Action action) const {
        const auto &b = m_bindings[action];
        const auto *window = m_window;

        if (m_active_device == ActiveDevice::KeyboardMouse) {
            if (b.kbm.mouse_axis.has_value()) {
                const f32 v = resolve_mouse_axis_raw(b) * b.kbm.mouse_sensitivity;
                return std::abs(v) >= b.kbm.mouse_threshold;
            }
            if (b.kbm.key.has_value() && window->is_key_pressed(*b.kbm.key)) {
                return true;
            }
            if (b.kbm.mouse_button.has_value() &&
                window->is_mouse_button_pressed(*b.kbm.mouse_button)) {
                return true;
            }
            if (b.kbm.axis_negative.has_value() && window->is_key_pressed(*b.kbm.axis_negative)) {
                return true;
            }
            if (b.kbm.axis_positive.has_value() && window->is_key_pressed(*b.kbm.axis_positive)) {
                return true;
            }
            return false;
        }

        if (b.controller.button.has_value() &&
            window->is_controller_button_pressed(*b.controller.button)) {
            return true;
        }
        if (b.controller.axis.has_value()) {
            const f32 v = window->get_controller_axis(*b.controller.axis);
            return std::abs(v) >= b.controller.axis_threshold;
        }
        return false;
    }

    bool InputSystem::resolve_just_pressed(Action action) const {
        const auto &b = m_bindings[action];
        const auto *window = m_window;

        if (m_active_device == ActiveDevice::KeyboardMouse) {
            // Mouse axes have no persistent state — any non-zero movement this frame IS "just pressed".
            if (b.kbm.mouse_axis.has_value()) {
                const f32 v = resolve_mouse_axis_raw(b) * b.kbm.mouse_sensitivity;
                return std::abs(v) >= b.kbm.mouse_threshold;
            }
            if (b.kbm.key.has_value() && window->is_key_just_pressed(*b.kbm.key)) {
                return true;
            }
            if (b.kbm.mouse_button.has_value() &&
                window->is_mouse_button_just_pressed(*b.kbm.mouse_button)) {
                return true;
            }
            if (b.kbm.axis_negative.has_value() &&
                window->is_key_just_pressed(*b.kbm.axis_negative)) {
                return true;
            }
            if (b.kbm.axis_positive.has_value() &&
                window->is_key_just_pressed(*b.kbm.axis_positive)) {
                return true;
            }
            return false;
        }

        if (b.controller.button.has_value()) {
            return window->is_controller_button_just_pressed(*b.controller.button);
        }
        return false;
    }

    bool InputSystem::resolve_just_released(Action action) const {
        const auto &b = m_bindings[action];
        const auto *window = m_window;

        if (m_active_device == ActiveDevice::KeyboardMouse) {
            if (b.kbm.mouse_axis.has_value()) {
                return false;
            }
            if (b.kbm.key.has_value() && window->is_key_just_released(*b.kbm.key)) {
                return true;
            }
            if (b.kbm.mouse_button.has_value() &&
                window->is_mouse_button_just_released(*b.kbm.mouse_button)) {
                return true;
            }
            if (b.kbm.axis_negative.has_value() &&
                window->is_key_just_released(*b.kbm.axis_negative)) {
                return true;
            }
            if (b.kbm.axis_positive.has_value() &&
                window->is_key_just_released(*b.kbm.axis_positive)) {
                return true;
            }
            return false;
        }

        if (b.controller.button.has_value()) {
            return window->is_controller_button_just_released(*b.controller.button);
        }
        return false;
    }

    f32 InputSystem::resolve_axis(Action action) const {
        const auto &b = m_bindings[action];
        const auto *window = m_window;

        if (m_active_device == ActiveDevice::KeyboardMouse) {
            if (b.kbm.mouse_axis.has_value()) {
                return resolve_mouse_axis_raw(b) * b.kbm.mouse_sensitivity;
            }

            if (b.kbm.axis_negative.has_value() || b.kbm.axis_positive.has_value()) {
                const f32 neg =
                    b.kbm.axis_negative.has_value() && window->is_key_pressed(*b.kbm.axis_negative)
                        ? -1.0f
                        : 0.0f;
                const f32 pos =
                    b.kbm.axis_positive.has_value() && window->is_key_pressed(*b.kbm.axis_positive)
                        ? 1.0f
                        : 0.0f;
                return neg + pos;
            }
            if (b.kbm.key.has_value()) {
                return window->is_key_pressed(*b.kbm.key) ? 1.0f : 0.0f;
            }
            if (b.kbm.mouse_button.has_value()) {
                return window->is_mouse_button_pressed(*b.kbm.mouse_button) ? 1.0f : 0.0f;
            }
            return 0.0f;
        }

        if (b.controller.axis.has_value()) {
            const f32 v = window->get_controller_axis(*b.controller.axis);
            return std::abs(v) < b.controller.deadzone ? 0.0f : v;
        }
        if (b.controller.button.has_value()) {
            return window->is_controller_button_pressed(*b.controller.button) ? 1.0f : 0.0f;
        }
        return 0.0f;
    }

} // namespace mantle
