// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace mantle {
    namespace Layers {
        constexpr JPH::ObjectLayer NON_MOVING = 0;
        constexpr JPH::ObjectLayer MOVING = 1;
        constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    } // namespace Layers

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
      public:
        bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;
    };
} // namespace mantle
