#pragma once

#if defined(__GNUC__) || defined(__clang__)
#define MANTLE_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define MANTLE_UNLIKELY(x) (x)
#endif