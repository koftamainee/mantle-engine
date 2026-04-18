#pragma once

#if defined(__GNUC__) || defined(__clang__)
#define MANTLE_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define MANTLE_UNLIKELY(x) (x)
#endif

#define MANTLE_NO_COPY(type)                                                   \
    type(const type &) = delete;                                               \
    type &operator=(const type &) = delete

#define MANTLE_NO_MOVE(type)                                                   \
    type(type &&) noexcept = delete;                                           \
    type &operator=(type &&) noexcept = delete

#define MANTLE_NO_COPY_NO_MOVE(type)                                           \
    MANTLE_NO_COPY(type);                                                      \
    MANTLE_NO_MOVE(type)
