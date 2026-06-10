// Copyright (c) 2026 Mantle. All rights reserved.

#include "layers.h"

namespace mantle {

    bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1,
                                                  JPH::ObjectLayer inObject2) const {
        switch (inObject1) {
            case Layers::NON_MOVING: {
                return inObject2 == Layers::MOVING;
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
