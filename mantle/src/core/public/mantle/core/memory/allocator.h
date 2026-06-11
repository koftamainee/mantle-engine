// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <concepts>
#include <string_view>

#include "mantle/core/memory/memory_block.h"
#include "mantle/core/types.h"

namespace mantle {

    template <typename A>
    concept CAllocator = (!std::copyable<A>) && requires(A a, void *ptr, usize size, usize align,
                                                         MemoryBlock block, std::string_view name) {
        { a.init(block, name) } -> std::same_as<void>;
        { a.destroy() } -> std::same_as<void>;

        { a.alloc(size, align) } -> std::same_as<void *>;
        { a.alloc(size) } -> std::same_as<void *>;
        { a.realloc(ptr, size) } -> std::same_as<void *>;
        { a.free(ptr) } -> std::same_as<void>;
    };

    template <typename A>
    concept CResetableAllocator = requires(A a) {
        { a.reset() } -> std::same_as<void>;
    };

    template <typename A, typename T>
    concept CTypedAllocator = CAllocator<A> && requires(A a, usize count) {
        { a.template alloc<T>() } -> std::same_as<T *>;
        { a.template alloc<T>(count) } -> std::same_as<T *>;
        { a.template alloc_zeroed<T>() } -> std::same_as<T *>;
        { a.template alloc_zeroed<T>(count) } -> std::same_as<T *>;
    };

    template <typename A, typename T, typename... Args>
    concept CEmplaceAllocator =
        CAllocator<A> && std::constructible_from<T, Args...> && requires(A a, Args &&...args) {
            { a.template emplace<T>(std::forward<Args>(args)...) } -> std::same_as<T *>;
        };

} // namespace mantle
