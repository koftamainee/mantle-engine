// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

// fix-includes off
#include <Jolt/Jolt.h>
// fix-include on

#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "arena_temp_allocator.h"
#include "mantle/core/memory/tlsf_allocator.h"
#include "mantle/physics/physics_system.h"
#include "layers.h"
#include "broad_phase_layers.h"
#include "mantle/core/memory/memory_units.h"

namespace mantle {

    struct PhysicsSystem::Impl final {
        ThreadSafeAllocator<TlsfAllocator> allocator {};
        spdlog::logger                    *logger = nullptr;

        ArenaTempAllocator temp_allocator {};
        // JPH::TempAllocatorImpl temp_allocator {megabytes(32)};

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
