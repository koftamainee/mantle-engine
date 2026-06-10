// Copyright (c) 2026 Mantle. All rights reserved.

#include "physics/physics_system.h"

#include <cstdarg>

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h"


#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>

#include "core/assert.h"
#include "core/memory/memory_units.h"

namespace mantle {
    namespace {
        spdlog::logger                     *s_logger = nullptr;
        ThreadSafeAllocator<TlsfAllocator> *s_allocator = nullptr;


        void jolt_trace(const char *fmt, ...) {
            va_list args;
            va_start(args, fmt);
            char buf[1024];
            vsnprintf(buf, sizeof(buf), fmt, args);
            va_end(args);

            if (s_logger) {
                s_logger->trace("{}", buf);
            }
        }

        JPH_IF_ENABLE_ASSERTS(bool jolt_assert_failed(const char *expr, const char *msg,
                                                      const char *file, uint line) {
            if (s_logger) {
                s_logger->critical("Jolt assert: {}:{}: {} ({})", file, line, msg ? msg : "", expr);
            }
            return true;
        })

        void *jolt_alloc(size_t size) { return s_allocator->alloc(size); }
        void  jolt_free(void *ptr) { s_allocator->free(ptr); }
        void *jolt_realloc(void *ptr, size_t old_size, size_t new_size) {
            return s_allocator->realloc(ptr, new_size);
        }
        void *jolt_aligned_alloc(size_t size, size_t align) {
            return s_allocator->alloc(size, align);
        }
        void jolt_aligned_free(void *ptr) { s_allocator->free(ptr); }

        namespace Layers {
            constexpr JPH::ObjectLayer NON_MOVING = 0;
            constexpr JPH::ObjectLayer MOVING = 1;
            constexpr JPH::ObjectLayer NUM_LAYERS = 2;
        } // namespace Layers

        class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
          public:
            bool ShouldCollide(JPH::ObjectLayer inObject1,
                               JPH::ObjectLayer inObject2) const override {
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


    } // anonymous namespace


    void PhysicsSystem::init(MemoryBlock block) {
        MANTLE_CHECK(!m_is_initialized);

        m_logger = spdlog::get("physics").get();
        s_logger = m_logger;

        JPH::Trace = jolt_trace;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = jolt_assert_failed);

        auto [tlsf_block, temp_block] = block.split<megabytes(10), megabytes(10)>();

        m_allocator.init(tlsf_block, "physics system general allocator");
        s_allocator = &m_allocator;

        JPH::Allocate = jolt_alloc;
        JPH::Reallocate = jolt_realloc;
        JPH::Free = jolt_free;
        JPH::AlignedAllocate = jolt_aligned_alloc;
        JPH::AlignedFree = jolt_aligned_free;

        JPH::Factory::sInstance = m_allocator.emplace<JPH::Factory>();

        JPH::RegisterTypes();

        m_temp_allocator.init(temp_block, "physics system temp allocator");

        constexpr u32 num_threads = 4;

        m_job_system = m_allocator.emplace<JPH::JobSystemThreadPool>(
            JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, num_threads);


        m_is_initialized = true;
        m_logger->info("Physics system initialized");
    }


    void PhysicsSystem::update(f32 dt) { MANTLE_CHECK(m_is_initialized); }

    void PhysicsSystem::destroy() {
        if (m_is_initialized) {
            JPH::UnregisterTypes();

            m_allocator.free(JPH::Factory::sInstance);
            JPH::Factory::sInstance = nullptr;

            m_job_system->~JobSystemThreadPool();
            m_allocator.free(m_job_system);

            JPH::Allocate = nullptr;
            JPH::Free = nullptr;
            JPH::Reallocate = nullptr;
            JPH::AlignedAllocate = nullptr;
            JPH::AlignedFree = nullptr;

            s_allocator = nullptr;
            s_logger = nullptr;

            m_allocator.destroy();
            m_temp_allocator.destroy();

            m_is_initialized = false;
            m_logger->info("Physics system destroyed");
        }
    }

} // namespace mantle
