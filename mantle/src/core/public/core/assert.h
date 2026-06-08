// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <cstdint>
#include <cstdlib>
#include <source_location>
#include <spdlog/spdlog.h>
#include <utility>

#include "macros.h"
#include "types.h"

namespace mantle {

#ifdef NDEBUG
#define DO_CHECK 0
#else
#define DO_CHECK 1
#endif

    [[noreturn]] inline void debug_break() {
#if defined(_MSC_VER)
        __debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
        __builtin_trap();
#else
        std::abort();
#endif
    }

    constexpr u32 fnv1a(const char *str, u32 line) {
        u32 h = 2166136261u ^ line;
        while (*str) {
            char c = *str;
            if (c == '\\') {
                c = '/';
            }
            h ^= static_cast<u8>(c);
            h *= 16777619u;
            ++str;
        }
        return h;
    }

    constexpr const char *project_filename(const char *path) {
        const char *found = nullptr;
        for (const char *p = path; *p; ++p) {
            if (p[0] == 'm' && p[1] == 'a' && p[2] == 'n' && p[3] == 't' && p[4] == 'l' &&
                p[5] == 'e') {
                found = p;
            }
        }
        return found ? found : path;
    }

    template <typename... Args>
    [[noreturn]] inline void debug_assert_failed(const char *expr, std::source_location loc,
                                                 const char *fmt, Args &&...args) {
        spdlog::critical("Assertion failed: ({})\n  File: {}\n  Line: {}\n  Function: {}", expr,
                         project_filename(loc.file_name()), loc.line(), loc.function_name());
        if constexpr (sizeof...(Args) > 0) {
            spdlog::critical(fmt, std::forward<Args>(args)...);
        }
        debug_break();
    }

    template <typename... Args>
    [[noreturn]] inline void debug_fatal_failed(u32 error_code, std::source_location loc,
                                                const char *fmt, Args &&...args) {
#if DO_CHECK
        spdlog::critical("Fatal error\n  File: {}\n  Line: {}\n  Function: "
                         "{}\n  Code: E{:08X}",
                         project_filename(loc.file_name()), loc.line(), loc.function_name(),
                         error_code);
        spdlog::critical(fmt, std::forward<Args>(args)...);
#else
        spdlog::critical("Fatal error [E{:08X}]", error_code);
#endif
        debug_break();
    }

    template <typename... Args>
    inline void debug_ensure_failed(const char *expr, std::source_location loc, const char *fmt,
                                    Args &&...args) {
        spdlog::warn("Ensure failed: ({})\n  File: {}\n  Line: {}\n  Function: {}", expr,
                     project_filename(loc.file_name()), loc.line(), loc.function_name());
        if constexpr (sizeof...(Args) > 0) {
            spdlog::warn(fmt, std::forward<Args>(args)...);
        }
    }

#if DO_CHECK

#define MANTLE_CHECK(expr)                                                                         \
    do {                                                                                           \
        if (MANTLE_UNLIKELY(!(expr))) {                                                            \
            ::mantle::debug_assert_failed(#expr, std::source_location::current(), "");             \
        }                                                                                          \
    } while (0)

#define MANTLE_CHECKF(expr, fmt, ...)                                                              \
    do {                                                                                           \
        if (MANTLE_UNLIKELY(!(expr))) {                                                            \
            ::mantle::debug_assert_failed(#expr, std::source_location::current(), fmt,             \
                                          ##__VA_ARGS__);                                          \
        }                                                                                          \
    } while (0)

#else

#define MANTLE_CHECK(expr) ((void)0)
#define MANTLE_CHECKF(expr, fmt, ...) ((void)0)

#endif

#define MANTLE_FATAL(expr, fmt, ...)                                                               \
    do {                                                                                           \
        if (MANTLE_UNLIKELY(expr)) {                                                               \
            constexpr auto          _loc = std::source_location::current();                        \
            constexpr ::mantle::u32 _code =                                                        \
                ::mantle::fnv1a(::mantle::project_filename(_loc.file_name()), _loc.line());        \
            ::mantle::debug_fatal_failed(_code, _loc, fmt, ##__VA_ARGS__);                         \
        }                                                                                          \
    } while (0)

#if DO_CHECK

#define MANTLE_ENSURE(expr)                                                                        \
    ([&]() -> bool {                                                                               \
        if (MANTLE_UNLIKELY(!(expr))) {                                                            \
            ::mantle::debug_ensure_failed(#expr, std::source_location::current(), "");             \
            return false;                                                                          \
        }                                                                                          \
        return true;                                                                               \
    })()

#define MANTLE_ENSUREF(expr, fmt, ...)                                                             \
    ([&]() -> bool {                                                                               \
        if (MANTLE_UNLIKELY(!(expr))) {                                                            \
            ::mantle::debug_ensure_failed(#expr, std::source_location::current(), fmt,             \
                                          ##__VA_ARGS__);                                          \
            return false;                                                                          \
        }                                                                                          \
        return true;                                                                               \
    })()

#else

#define MANTLE_ENSURE(expr) (!!(expr))
#define MANTLE_ENSUREF(expr, fmt, ...) (!!(expr))

#endif

} // namespace mantle
