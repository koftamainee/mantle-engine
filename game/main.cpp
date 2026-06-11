#include "mantle/core/types.h"
#include "mantle/engine/engine.h"
#include "mantle/engine/script.h"
#include "mantle/ecs/ecs.h"
#include "mantle/ecs/components.h"
#include "mantle/input/input.h"
#include "mantle/window/window.h"

#include "scripts/player.h"
#include "scripts/camera.h"

void setup_actions() {
    using Key = mantle::Window::Key;
    using CButton = mantle::Window::ControllerButton;
    using CAxis = mantle::Window::ControllerAxis;

    auto &input = mantle::Input::get();

    input.add_action("move_left");
    input.bind_key("move_left", static_cast<mantle::u16>(Key::A));
    input.bind_controller_axis("move_left", static_cast<mantle::u8>(CAxis::LeftX));

    input.add_action("move_right");
    input.bind_key("move_right", static_cast<mantle::u16>(Key::D));
    input.bind_controller_axis("move_right", static_cast<mantle::u8>(CAxis::LeftX));

    input.add_action("move_forward");
    input.bind_key("move_forward", static_cast<mantle::u16>(Key::W));
    input.bind_controller_axis("move_forward", static_cast<mantle::u8>(CAxis::LeftY));

    input.add_action("move_backward");
    input.bind_key("move_backward", static_cast<mantle::u16>(Key::S));
    input.bind_controller_axis("move_backward", static_cast<mantle::u8>(CAxis::LeftY));

    input.add_action("jump");
    input.bind_key("jump", static_cast<mantle::u16>(Key::Space));
    input.bind_controller_button("jump", static_cast<mantle::u8>(CButton::A));

    input.add_action("sprint");
    input.bind_key("sprint", static_cast<mantle::u16>(Key::LControl));
    input.bind_controller_button("sprint", static_cast<mantle::u8>(CButton::B));

    input.add_action("look_right");
    input.add_action("look_left");
}

int main() {
    mantle::EngineConfig cfg;
    cfg.window = {.title = "Mantle Game", .size = {1920, 1080}, .fullscreen = false};

    mantle::Engine engine;
    engine.init(cfg);

    setup_actions();

    auto &ecs = engine.ecs();

    ecs.create_entity("Floor").add<mantle::MeshComponent>(0u);

    ecs.create_entity("Capsule")
        .set<mantle::Transform>({.position = {0.0f, 5.0f + 0.8f, 0.0f}})
        .add<mantle::MeshComponent>(1u)
        .set<mantle::ScriptComponent>({.callbacks = &game::player_script});

    ecs.create_entity("Camera")
        .set<mantle::Transform>({.position = {0.0f, 3.0f, 8.0f}})
        .set<mantle::Camera>({.fov = 75.0f, .aspect = 1920.0f / 1080.0f})
        .set<mantle::ScriptComponent>({.callbacks = &game::camera_script});

    engine.run();
    engine.destroy();

    return 0;
}
