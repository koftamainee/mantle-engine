// Copyright (c) 2026 Mantle. All rights reserved.

#include "physics/physics_system.h"

#include <cstdarg>

#include "Jolt/Core/Factory.h"
#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "core/assert.h"

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
        void *jolt_aligned_alloc(size_t size, size_t align) {
            return s_allocator->alloc(size, align);
        }
        void jolt_aligned_free(void *ptr) { s_allocator->free(ptr); }
    } // anonymous namespace


    void PhysicsSystem::init(MemoryBlock block) {
        MANTLE_CHECK(!m_is_initialized);

        m_logger = spdlog::get("physics").get();
        s_logger = m_logger;

        JPH::Trace = jolt_trace;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = jolt_assert_failed);

        m_allocator.init(block, "physics system general allocator");
        s_allocator = &m_allocator;

        JPH::Allocate = jolt_alloc;
        JPH::Free = jolt_free;
        JPH::AlignedAllocate = jolt_aligned_alloc;
        JPH::AlignedFree = jolt_aligned_free;

        JPH::Factory::sInstance = m_allocator.emplace<JPH::Factory>();

        JPH::RegisterTypes();

        m_is_initialized = true;
        m_logger->info("Physics system initialized");
    }


    void PhysicsSystem::update(f32 dt) { MANTLE_CHECK(m_is_initialized); }


    void PhysicsSystem::destroy() {
        if (m_is_initialized) {
            JPH::UnregisterTypes();

            m_allocator.free(JPH::Factory::sInstance);
            JPH::Factory::sInstance = nullptr;

            JPH::Allocate = nullptr;
            JPH::Free = nullptr;
            JPH::AlignedAllocate = nullptr;
            JPH::AlignedFree = nullptr;

            s_allocator = nullptr;
            s_logger = nullptr;

            m_allocator.destroy();

            m_is_initialized = false;
            m_logger->info("Physics system destroyed");
        }
    }

} // namespace mantle
