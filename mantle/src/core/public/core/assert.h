#pragma once
#include <cstdlib>
#include <cstdint>
#include <utility>

#include "spdlog/spdlog.h"
#include "macros.h"

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

template<typename... Args>
[[noreturn]] inline void debug_assert_failed(const char* expr,
                                             const char* file,
                                             int32_t line,
                                             const char* fmt,
                                             Args&&... args) {
    spdlog::critical("Assertion failed: ({})\n  File: {}\n  Line: {}", expr, file, line);
    if constexpr (sizeof...(Args) > 0) {
        spdlog::critical(fmt, std::forward<Args>(args)...);
    }
    debug_break();
    std::abort();
}

template<typename... Args>
[[noreturn]] inline void debug_fatal_failed(const char* file,
                                            int32_t line,
                                            const char* fmt,
                                            Args&&... args) {
    spdlog::critical("Fatal error\n  File: {}\n  Line: {}", file, line);
    spdlog::critical(fmt, std::forward<Args>(args)...);
    debug_break();
    std::abort();
}

template<typename... Args>
inline void debug_ensure_failed(const char* expr,
                               const char* file,
                               int32_t line,
                               const char* fmt,
                               Args&&... args) {
    spdlog::warn("Ensure failed: ({})\n  File: {}\n  Line: {}", expr, file, line);
    if constexpr (sizeof...(Args) > 0) {
        spdlog::warn(fmt, std::forward<Args>(args)...);
    }
}

#if DO_CHECK

#define check(expr) \
    do { \
        if (MANTLE_UNLIKELY(!(expr))) { \
            ::mantle::debug_assert_failed(#expr, __FILE__, __LINE__, ""); \
        } \
    } while (0)

#define checkf(expr, fmt, ...) \
    do { \
        if (MANTLE_UNLIKELY(!(expr))) { \
            ::mantle::debug_assert_failed(#expr, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#else

#define check(expr) ((void)0)
#define checkf(expr, fmt, ...) ((void)0)

#endif

#define fatal(expr, fmt, ...) \
    do { \
        if (MANTLE_UNLIKELY(expr)) { \
            ::mantle::debug_fatal_failed(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#if DO_CHECK

#define ensure(expr) \
    ([&]() -> bool { \
        if (MANTLE_UNLIKELY(!(expr))) { \
            ::mantle::debug_ensure_failed(#expr, __FILE__, __LINE__, ""); \
            return false; \
        } \
        return true; \
    })()

#define ensuref(expr, fmt, ...) \
    ([&]() -> bool { \
        if (MANTLE_UNLIKELY(!(expr))) { \
            ::mantle::debug_ensure_failed(#expr, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
            return false; \
        } \
        return true; \
    })()

#else

#define ensure(expr) (!!(expr))
#define ensuref(expr, fmt, ...) (!!(expr))

#endif

} // namespace mantle