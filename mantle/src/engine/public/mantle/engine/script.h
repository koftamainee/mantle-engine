// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "mantle/core/types.h"

namespace mantle {
    class Engine;
    class Entity;

    enum class ScriptPhase {
        PhysicsUpdate,
        Update,
        LateUpdate,
    };

    using ScriptFn   = void (*)(Engine &engine);
    using ScriptDtFn = void (*)(Engine &engine, f32 dt);

    struct ScriptCallbacks {
        ScriptFn   on_awake          = nullptr;
        ScriptFn   on_start          = nullptr;
        ScriptDtFn on_update         = nullptr;
        ScriptDtFn on_physics_update = nullptr;
        ScriptDtFn on_late_update    = nullptr;
        ScriptFn   on_destroy        = nullptr;
    };

    struct ScriptComponent {
        bool                   ready_called = false;
        const ScriptCallbacks *callbacks    = nullptr;
    };
} // namespace mantle
