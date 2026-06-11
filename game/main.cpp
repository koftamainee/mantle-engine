#include "actions.h"
#include "flecs.h"
#include "mantle/core/types.h"
#include "mantle/ecs/components.h"
#include "mantle/engine/engine.h"
#include "mantle/window/window.h"

using namespace mantle;
using namespace game;


void register_inputs(InputSystem &input) {
    input.bind_axis(GameAction::MoveX, Key::W, Key::S);
    input.bind_axis(GameAction::MoveX, ControllerAxis::LeftX);
    input.bind_axis(GameAction::MoveY, Key::D, Key::A);
    input.bind_axis(GameAction::MoveY, ControllerAxis::LeftY);

    input.bind(GameAction::Jump, Key::Space);
    input.bind(GameAction::Jump, ControllerButton::A);
}

int main() {
    EngineConfig cfg;
    cfg.window = {.title = "Mantle Engine", .size = {0, 0}, .fullscreen = true};

    Engine engine;
    engine.init(cfg);

    register_inputs(engine.input_system());


    flecs::world &world = engine.world();


    engine.run();
    engine.destroy();


    return 0;
}
