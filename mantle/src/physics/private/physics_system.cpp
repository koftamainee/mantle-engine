// Copyright (c) 2026 Mantle. All rights reserved.

#include "physics/physics_system.h"

#include "core/assert.h"
#include "Jolt/Jolt.h"

#include <cstdarg>

namespace mantle {
    namespace {
        spdlog::logger *s_logger = nullptr;

        void trace_jolt(const char *fmt, ...) {
            va_list args;
            va_start(args, fmt);
            char buf[1024];
            vsnprintf(buf, sizeof(buf), fmt, args);
            va_end(args);

            if (s_logger) {
                s_logger->trace("{}", buf);
            }
        }

        bool assert_failed_jolt(const char *expr, const char *msg,
                                const char *file, uint line) {
            if (s_logger) {
                s_logger->critical("Jolt assert: {}:{}: {} ({})", file, line,
                                   msg ? msg : "", expr);
            }
            return true;
        }
    } // anonymous namespace


    void PhysicsSystem::init(MemoryBlock block) {
        MANTLE_CHECK(!m_is_initialized);

        m_logger = spdlog::get("physics").get();
        s_logger = m_logger;

        JPH::Trace = trace_jolt;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = assert_failed_jolt);


        m_is_initialized = true;
        m_logger->info("Physics system initialized");
    }


    void PhysicsSystem::update(f32 dt) {
        MANTLE_CHECK(m_is_initialized);
    }


    void PhysicsSystem::destroy() {
        if (m_is_initialized) {
            m_is_initialized = false;
            m_logger->info("Physics system destroyed");
        }
    }

} // namespace mantle
