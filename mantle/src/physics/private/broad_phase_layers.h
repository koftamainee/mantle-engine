// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "mantle/core/types.h"
#include "layers.h"

namespace mantle {
    namespace BroadPhaseLayers {
        constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        constexpr JPH::BroadPhaseLayer MOVING(1);
        constexpr u32                  NUM_LAYERS(2);
    } // namespace BroadPhaseLayers

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
      public:
        BPLayerInterfaceImpl();

        JPH::uint GetNumBroadPhaseLayers() const override;

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

      private:
        JPH::BroadPhaseLayer m_object_to_broad_phase[Layers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
      public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
    };

} // namespace mantle
