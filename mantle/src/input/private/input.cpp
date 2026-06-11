#include "mantle/input/input.h"

#include <algorithm>
#include <cmath>

#include "mantle/core/assert.h"
#include "mantle/window/window.h"

namespace mantle {
    Input *Input::s_instance = nullptr;

    void Input::init(MemoryBlock block) {
        MANTLE_CHECK(!s_instance);
        s_instance = new Input();
        s_instance->m_allocator.init(block, "input");
        s_instance->m_resource = TlsfResource(&s_instance->m_allocator);
    }

    void Input::shutdown() {
        if (s_instance) {
            delete s_instance;
            s_instance = nullptr;
        }
    }

    Input &Input::get() {
        MANTLE_CHECK(s_instance);
        return *s_instance;
    }

    void Input::begin_frame(Window &window) {
        auto &inst = get();
        inst.m_window = &window;
        inst.m_queried_this_frame = false;

        for (auto &[name, state] : inst.m_states) {
            state.pressed_prev = state.pressed;
        }

        for (auto &[name, action] : inst.m_actions) {
            inst.update_action_state(name, action, window);
        }
    }

    bool Input::is_action_pressed(std::string_view name) {
        auto &inst = get();
        auto  it = inst.m_states.find(name);
        return it != inst.m_states.end() && it->second.pressed;
    }

    bool Input::is_action_just_pressed(std::string_view name) {
        auto &inst = get();
        auto  it = inst.m_states.find(name);
        return it != inst.m_states.end() && it->second.pressed && !it->second.pressed_prev;
    }

    bool Input::is_action_just_released(std::string_view name) {
        auto &inst = get();
        auto  it = inst.m_states.find(name);
        return it != inst.m_states.end() && !it->second.pressed && it->second.pressed_prev;
    }

    f32 Input::get_action_strength(std::string_view name) {
        auto &inst = get();
        auto  it = inst.m_states.find(name);
        return it != inst.m_states.end() ? it->second.value : 0.0f;
    }

    f32 Input::get_axis(std::string_view negative, std::string_view positive) {
        return get_action_strength(positive) - get_action_strength(negative);
    }

    glm::vec2 Input::get_vector(std::string_view negative_x, std::string_view positive_x,
                                std::string_view negative_y, std::string_view positive_y) {
        glm::vec2 v(get_axis(negative_x, positive_x), get_axis(negative_y, positive_y));
        f32       len = glm::length(v);
        if (len > 1.0f) {
            v /= len;
        }
        return v;
    }

    void Input::add_action(std::string_view name, f32 deadzone, InputActionType type) {
        auto it = m_actions.find(name);
        if (it != m_actions.end()) {
            it->second.deadzone = deadzone;
            it->second.type = type;
            return;
        }
        std::pmr::string key(name, &m_resource);
        it = m_actions.emplace(std::move(key), InputAction {}).first;
        it->second.deadzone = deadzone;
        it->second.type = type;
    }

    void Input::bind_key(std::string_view action_name, u16 key) {
        auto it = m_actions.find(action_name);
        if (it != m_actions.end() && it->second.key_count < 8) {
            it->second.keys[it->second.key_count++] = key;
        }
    }

    void Input::bind_mouse_button(std::string_view action_name, u8 button) {
        auto it = m_actions.find(action_name);
        if (it != m_actions.end() && it->second.mouse_count < 8) {
            it->second.mouse_buttons[it->second.mouse_count++] = button;
        }
    }

    void Input::bind_controller_button(std::string_view action_name, u8 button) {
        auto it = m_actions.find(action_name);
        if (it != m_actions.end() && it->second.ctrl_btn_count < 8) {
            it->second.controller_buttons[it->second.ctrl_btn_count++] = button;
        }
    }

    void Input::bind_controller_axis(std::string_view action_name, u8 axis) {
        auto it = m_actions.find(action_name);
        if (it != m_actions.end() && it->second.ctrl_axis_count < 8) {
            it->second.controller_axes[it->second.ctrl_axis_count++] = axis;
        }
    }

    void Input::start_vibration(u16 low, u16 high, u32 duration_ms) {
        if (m_window) {
            m_window->set_controller_rumble(low, high, duration_ms);
        }
    }

    void Input::stop_vibration() {
        if (m_window) {
            m_window->stop_controller_rumble();
        }
    }

    void Input::update_action_state(std::string_view name, InputAction &action,
                                    const Window &window) {
        auto [it, inserted] = m_states.emplace(
            std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple());
        auto &state = it->second;

        f32 max_value = 0.0f;

        for (u32 i = 0; i < action.key_count; i++) {
            if (window.is_key_pressed(static_cast<Window::Key>(action.keys[i]))) {
                max_value = std::max(max_value, 1.0f);
            }
        }

        for (u32 i = 0; i < action.mouse_count; i++) {
            if (window.is_mouse_button_pressed(
                    static_cast<Window::MouseButton>(action.mouse_buttons[i]))) {
                max_value = std::max(max_value, 1.0f);
            }
        }

        for (u32 i = 0; i < action.ctrl_btn_count; i++) {
            if (window.is_controller_button_pressed(
                    static_cast<Window::ControllerButton>(action.controller_buttons[i]))) {
                max_value = std::max(max_value, 1.0f);
            }
        }

        for (u32 i = 0; i < action.ctrl_axis_count; i++) {
            f32 raw = window.get_controller_axis(
                static_cast<Window::ControllerAxis>(action.controller_axes[i]));
            f32 abs_raw = std::abs(raw);
            max_value = std::max(max_value, abs_raw);
        }

        f32 dz = action.deadzone;
        f32 value = 0.0f;
        if (max_value > dz) {
            value = (max_value - dz) / (1.0f - dz);
        }

        state.value = value;
        state.pressed = value > 0.0f;
    }
} // namespace mantle
