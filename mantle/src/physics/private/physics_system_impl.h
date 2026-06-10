#pragma once
#include "arena_temp_allocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "core/memory/tlsf_allocator.h"
#include "physics/physics_system.h"

namespace mantle {
    namespace Layers {
        constexpr JPH::ObjectLayer NON_MOVING = 0;
        constexpr JPH::ObjectLayer MOVING = 1;
        constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    } // namespace Layers

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
      public:
        bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
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
    };

    namespace BroadPhaseLayers {
        constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        constexpr JPH::BroadPhaseLayer MOVING(1);
        constexpr u32                  NUM_LAYERS(2);
    } // namespace BroadPhaseLayers

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
      public:
        BPLayerInterfaceImpl() {
            m_object_to_broad_phase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            m_object_to_broad_phase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        }

        uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
            JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
            return m_object_to_broad_phase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
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

      private:
        JPH::BroadPhaseLayer m_object_to_broad_phase[Layers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
      public:
        bool ShouldCollide(JPH::ObjectLayer     inLayer1,
                           JPH::BroadPhaseLayer inLayer2) const override {
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
    };

    struct PhysicsSystem::Impl final {
        ThreadSafeAllocator<TlsfAllocator> allocator {};
        spdlog::logger                    *logger = nullptr;

        ArenaTempAllocator temp_allocator {};

        JPH::JobSystemThreadPool job_system {};

        BPLayerInterfaceImpl              broad_phase_layer_interface;
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
        ObjectLayerPairFilterImpl         object_vs_object_layer_filter;
        JPH::PhysicsSystem                physics_system;

        constexpr static u32 kCollisionSteps = 1;
        constexpr static u32 kMaxBodies = 1024;
        constexpr static u32 kNumBodyMutexes = 0;
        constexpr static u32 kMaxBodyPairs = 1024;
        constexpr static u32 kMaxContactConstraints = 1024;

        explicit Impl(MemoryBlock mem);
        ~Impl();
    };
} // namespace mantle
