// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/build_info/build_info.h"

#ifndef MANTLE_VERSION
#define MANTLE_VERSION "unknown"
#endif

#ifndef MANTLE_GIT_HASH
#define MANTLE_GIT_HASH "unknown"
#endif

#ifndef MANTLE_BUILD_TYPE
#define MANTLE_BUILD_TYPE "unknown"
#endif

#ifndef MANTLE_BUILD_DATE
#define MANTLE_BUILD_DATE "unknown"
#endif

#ifndef MANTLE_COMPILER
#define MANTLE_COMPILER "unknown"
#endif

namespace mantle {

    const char *version_string() { return "Mantle v" MANTLE_VERSION " (" MANTLE_GIT_HASH ")"; }

    const char *build_string() {
        return "Mantle v" MANTLE_VERSION " (" MANTLE_GIT_HASH ") | " MANTLE_BUILD_TYPE
               " | " MANTLE_COMPILER " | "
               "C++23";
    }
} // namespace mantle
