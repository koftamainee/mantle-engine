// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/system_info/system_info.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>

#include <spdlog/spdlog.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
// clang-format formats headers so windows h will be last and not include some macro
// clang-format off
#include <windows.h>
#include <intrin.h>
#include <sysinfoapi.h>
// clang-format on
#elif __linux__
#include <cpuid.h>
#include <fstream>
#include <string_view>
#include <sys/utsname.h>
#else
#include <string_view>
#endif

namespace mantle {

    static void log_os_info(spdlog::logger *logger) {
#ifdef _WIN32
        OSVERSIONINFOEXW osvi {};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
#pragma warning(suppress : 4996)
        GetVersionExW(reinterpret_cast<OSVERSIONINFOW *>(&osvi));
        logger->info("OS: Windows {}.{} (build {})", osvi.dwMajorVersion, osvi.dwMinorVersion,
                     osvi.dwBuildNumber);
#elif __linux__
        struct utsname uts {};
        if (uname(&uts) == 0) {
            logger->info("OS: {} {} ({})", uts.sysname, uts.release, uts.machine);
        } else {
            logger->warn("OS: failed to query uname");
        }
        std::ifstream f("/etc/os-release");
        std::string   line;
        while (std::getline(f, line)) {
            if (line.starts_with("PRETTY_NAME=")) {
                auto val = line.substr(12);
                if (!val.empty() && val.front() == '"') {
                    val = val.substr(1);
                }
                if (!val.empty() && val.back() == '"') {
                    val.pop_back();
                }
                logger->info("Distro: {}", val);
                break;
            }
        }
#endif
    }

    static std::string get_cpu_name() {
        char         brand[49] = {};
        unsigned int regs[4];
        for (unsigned int i = 0; i < 3; ++i) {
#ifdef _WIN32
            __cpuid(reinterpret_cast<int *>(regs), 0x80000002 + i);
#else
            __cpuid(0x80000002 + i, regs[0], regs[1], regs[2], regs[3]);
#endif
            std::memcpy(brand + i * 16, regs, 16);
        }
        std::string result(brand);
        auto        start = result.find_first_not_of(' ');
        return start != std::string::npos ? result.substr(start) : result;
    }

    static void log_cpu_info(spdlog::logger *logger) {
        const std::string name = get_cpu_name();
        const uint32_t    cores = std::thread::hardware_concurrency();
        logger->info("CPU: {} | {} logical cores", name, cores);
    }

    struct MemInfo {
        uint64_t total_bytes;
        uint64_t available_bytes;
    };

    static MemInfo get_mem_info() {
#ifdef _WIN32
        MEMORYSTATUSEX ms {};
        ms.dwLength = sizeof(ms);
        GlobalMemoryStatusEx(&ms);
        return {ms.ullTotalPhys, ms.ullAvailPhys};
#elif __linux__
        MemInfo       info {};
        std::ifstream f("/proc/meminfo");
        std::string   line;
        while (std::getline(f, line)) {
            const char *s = line.c_str();
            while (*s == ' ') {
                s++;
            }
            if (std::strncmp(s, "MemTotal:", 9) == 0) {
                while (*s && !(*s >= '0' && *s <= '9')) {
                    s++;
                }
                info.total_bytes = std::atoll(s) * 1024;
            } else if (std::strncmp(s, "MemAvailable:", 13) == 0) {
                while (*s && !(*s >= '0' && *s <= '9')) {
                    s++;
                }
                info.available_bytes = std::atoll(s) * 1024;
            }
        }
        return info;
#endif
    }

    static void log_memory_info(spdlog::logger *logger) {
        auto [total, avail] = get_mem_info();
        auto to_gb = [](uint64_t bytes) {
            return static_cast<float>(bytes) / (1024.0f * 1024.0f * 1024.0f);
        };
        logger->info("RAM: {:.1f} / {:.1f} GB available", to_gb(avail), to_gb(total));
    }

    void log_system_info(spdlog::logger *logger, std::string_view gpu_name, uint64_t vram_bytes,
                         u32 window_w, u32 window_h, bool vsync, f32 refresh_rate,
                         bool fullscreen) {
        log_os_info(logger);
        log_cpu_info(logger);
        log_memory_info(logger);
        if (!gpu_name.empty()) {
            logger->info("GPU: {} | {} MB VRAM", gpu_name, vram_bytes / (1024 * 1024));
        }
        if (window_w > 0 && window_h > 0) {
            logger->info("Window: {}x{} | {} | {} Hz{}", window_w, window_h,
                         fullscreen ? "Fullscreen" : "Windowed", static_cast<u32>(refresh_rate),
                         vsync ? " | V-Sync" : "");
        }
        logger->info("");
    }

} // namespace mantle
