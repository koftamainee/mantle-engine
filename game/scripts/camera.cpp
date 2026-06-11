#include "camera.h"

#include <glm/glm.hpp>

#include "mantle/camera/math.h"
#include "mantle/ecs/ecs.h"
#include "mantle/ecs/components.h"
#include "mantle/engine/engine.h"
#include "mantle/input/input.h"
#include "mantle/window/window.h"

#include "../components.h"

namespace game {
    using namespace mantle;

    namespace {
        constexpr f32 mouse_sensitivity = 0.5f;
        constexpr f32 controller_look_sensitivity = 120.0f;

        void on_physics_update(Engine &engine, f32 dt) {
            auto &window = engine.window();
            auto &ecs = engine.ecs();
            auto &cam_state = ecs.ensure_singleton<CameraState>();

            f32 look_dx = window.get_mouse_delta().x * mouse_sensitivity;
            f32 look_dy = window.get_mouse_delta().y * mouse_sensitivity;

            f32 rx = Input::get_action_strength("look_right");
            f32 lx = Input::get_action_strength("look_left");
            if (rx != 0.0f || lx != 0.0f) {
                look_dx = (rx - lx) * controller_look_sensitivity * dt;
            }

            cam_state.yaw += look_dx;
            cam_state.pitch = glm::clamp(cam_state.pitch - look_dy, -89.0f, 89.0f);

            auto &character = engine.character();
            glm::vec3 target = character.get_position();

            Entity camera_entity = ecs.lookup("Camera");
            if (!camera_entity.is_alive()) {
                return;
            }

            glm::vec3 pos = orbit_position(target, cam_state.orbit_distance,
                                            cam_state.yaw, cam_state.pitch);
            auto *t = camera_entity.get_mut<Transform>();
            t->position = pos;
            t->rotation = glm::vec3(cam_state.pitch, cam_state.yaw, 0.0f);
        }
    }

    const ScriptCallbacks camera_script = {
        .on_physics_update = on_physics_update,
    };
}
