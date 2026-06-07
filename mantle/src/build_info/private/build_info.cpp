#include "build_info/build_info.h"

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

    const char *version_string() {
        return "Mantle v" MANTLE_VERSION " (" MANTLE_GIT_HASH ")";
    }

    const char *build_string() {
        return "Mantle v" MANTLE_VERSION " (" MANTLE_GIT_HASH ") | "
            MANTLE_BUILD_TYPE " | " MANTLE_COMPILER " | "
#if __cplusplus >= 202400L
            "C++26"
#elif __cplusplus >= 202302L
            "C++23"
#elif __cplusplus >= 202002L
            "C++20"
#elif __cplusplus >= 201703L
            "C++17"
#elif __cplusplus >= 201402L
            "C++14"
#elif __cplusplus >= 201103L
            "C++11"
#else
            "C++98"
#endif
            ;
    }
}
