// Copyright (c) 2026 Mantle. All rights reserved.
#pragma once

#include "core/types.h"
#include "flecs.h"

namespace mantle {
    class CharacterController;
    void bootstrap_player(const flecs::world &world, CharacterController &character);
} // namespace mantle