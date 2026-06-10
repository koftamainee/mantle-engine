// Copyright (c) 2026 Mantle. All rights reserved.

#include "physics_system_impl.h"

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/RegisterTypes.h>

#include <array>
#include <cstdarg>

#include "core/memory/memory_units.h"
#include "core/memory/thread_safe_allocator.h"
#include "core/memory/tlsf_allocator.h"

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
            auto *new_ptr = jolt_alloc(new_size);
            memcpy(new_ptr, ptr, std::min(old_size, new_size));
            jolt_free(ptr);
            return new_ptr;
        }
        void *jolt_aligned_alloc(size_t size, size_t align) {
            return s_allocator->alloc(size, align);
        }
        void jolt_aligned_free(void *ptr) { s_allocator->free(ptr); }
    } // namespace

    PhysicsSystem::Impl::Impl(MemoryBlock mem) {
        logger = spdlog::get("physics").get();
        JPH::Trace = jolt_trace;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = jolt_assert_failed);

        auto [tlsf_block, temp_block] = mem.split<megabytes(64), megabytes(32)>();

        allocator.init(tlsf_block, "physics system general allocator");
        s_allocator = &allocator;


        JPH::Allocate = jolt_alloc;
        JPH::Reallocate = jolt_realloc;
        JPH::Free = jolt_free;
        JPH::AlignedAllocate = jolt_aligned_alloc;
        JPH::AlignedFree = jolt_aligned_free;

        JPH::Factory::sInstance = allocator.emplace<JPH::Factory>();

        JPH::RegisterTypes();

        temp_allocator.init(temp_block, "physics system temp allocator");

        const u32 num_threads = std::thread::hardware_concurrency() - 1;

        new (&job_system)
            JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, num_threads);

        physics_system.Init(kMaxBodies, kNumBodyMutexes, kMaxBodyPairs, kMaxContactConstraints,
                            broad_phase_layer_interface, object_vs_broadphase_layer_filter,
                            object_vs_object_layer_filter);
    }

    PhysicsSystem::Impl::~Impl() {
        JPH::UnregisterTypes();

        allocator.free(JPH::Factory::sInstance);
        JPH::Factory::sInstance = nullptr;

        // cant destroy allocators, need to call to JPH physics system first
        // this fall backs to destructors and call them in reverse order of struct declaration so its fine
    }
} // namespace mantle
