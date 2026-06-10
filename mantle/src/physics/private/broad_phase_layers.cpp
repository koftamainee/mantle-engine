// Copyright (c) 2026 Mantle. All rights reserved.

#include "broad_phase_layers.h"

namespace mantle {

    BPLayerInterfaceImpl::BPLayerInterfaceImpl() {
        m_object_to_broad_phase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        m_object_to_broad_phase[Layers::MOVING] = BroadPhaseLayers::MOVING;
    }
    JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const {
        return BroadPhaseLayers::NUM_LAYERS;
    }
    JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return m_object_to_broad_phase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char *BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const {
        switch ((JPH::BroadPhaseLayer::Type)inLayer) {
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                return "MOVING";
            default:
                JPH_ASSERT(false);
                return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer     inLayer1,
                                                          JPH::BroadPhaseLayer inLayer2) const {
        switch (inLayer1) {
            case Layers::NON_MOVING: {
                return inLayer2 == BroadPhaseLayers::MOVING;
            }
            case Layers::MOVING: {
                return true;
            }
            default:
                JPH_ASSERT(false);
                return false;
        }
    }

} // namespace mantle
