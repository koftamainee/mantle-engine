// Copyright (c) 2026 Mantle. All rights reserved.

#include "input/input.h"

#include "components.h"
#include "window/window.h"

namespace mantle {
    namespace {
        constexpr f32 stick_deadzone = 0.15f;
        constexpr f32 trigger_deadzone = 0.1f;
        constexpr f32 controller_look_sensitivity = 120.0f;
        constexpr f32 mouse_sensitivity = 0.5f;

        f32 apply_deadzone(f32 value, f32 deadzone) {
            if (std::abs(value) < deadzone) {
                return 0.0f;
            }
            return value;
        }
    } // namespace

    void bootstrap_input(const flecs::world &world, Window &window) {
        (void)world.component<InputState>();
        world.set<InputState>({});

        world.system<>("Input").kind(flecs::PreUpdate).run([&window](const flecs::iter &it) {
            auto  esc_world = it.world();
            auto  dt = it.delta_time();
            auto &player_input = esc_world.ensure<InputState>();

            player_input = {};

            bool controller_active = false;

            f32 rx = apply_deadzone(window.get_controller_axis(Window::ControllerAxis::RightX),
                                    stick_deadzone);
            f32 ry = apply_deadzone(window.get_controller_axis(Window::ControllerAxis::RightY),
                                    stick_deadzone);
            if (rx != 0.0f || ry != 0.0f) {
                player_input.look_dx = rx * controller_look_sensitivity * dt;
                player_input.look_dy = -ry * controller_look_sensitivity * dt;
                controller_active = true;
            }

            f32 lx = apply_deadzone(window.get_controller_axis(Window::ControllerAxis::LeftX),
                                    stick_deadzone);
            f32 ly = apply_deadzone(window.get_controller_axis(Window::ControllerAxis::LeftY),
                                    stick_deadzone);
            if (lx != 0.0f || ly != 0.0f) {
                player_input.move_strafe = lx;
                player_input.move_forward = -ly;
                controller_active = true;
            }

            f32 lt = apply_deadzone(window.get_controller_axis(Window::ControllerAxis::LeftTrigger),
                                    trigger_deadzone);
            f32 rt = apply_deadzone(
                window.get_controller_axis(Window::ControllerAxis::RightTrigger), trigger_deadzone);
            if (lt != 0.0f || rt != 0.0f) {
                player_input.move_up = rt - lt;
                controller_active = true;
            }

            if (window.is_controller_button_pressed(Window::ControllerButton::B)) {
                player_input.sprint = true;
                controller_active = true;
            }

            if (window.is_controller_button_pressed(Window::ControllerButton::A)) {
                player_input.jump = true;
                controller_active = true;
            }

            Window::MouseDelta md = window.get_mouse_delta();
            player_input.look_dx += md.x * mouse_sensitivity;
            player_input.look_dy += md.y * mouse_sensitivity;
            player_input.controller_active = controller_active;

            if (!controller_active) {
                if (window.is_key_pressed(Window::Key::W)) {
                    player_input.move_forward += 1.0f;
                }
                if (window.is_key_pressed(Window::Key::S)) {
                    player_input.move_forward -= 1.0f;
                }
                if (window.is_key_pressed(Window::Key::A)) {
                    player_input.move_strafe -= 1.0f;
                }
                if (window.is_key_pressed(Window::Key::D)) {
                    player_input.move_strafe += 1.0f;
                }
                if (window.is_key_just_pressed(Window::Key::Space)) {
                    player_input.jump = true;
                }


                player_input.sprint =
                    player_input.sprint || window.is_key_pressed(Window::Key::LControl);
            }
        });
    }
} // namespace mantle
