// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <vulkan/vulkan.h>

#include "mantle/core/macros.h"
#include "mantle/core/memory/tlsf_allocator.h"

namespace mantle {

    class VulkanCPUAllocator final {
      public:
        VulkanCPUAllocator() = default;
        ~VulkanCPUAllocator() = default;

        MANTLE_NO_COPY_NO_MOVE(VulkanCPUAllocator);

        void init(TlsfAllocator *tlsf);

        VkAllocationCallbacks *vk_allocator();

      private:
        static void *VKAPI_CALL vk_alloc(void *user, usize size, usize align,
                                         VkSystemAllocationScope scope);

        static void *VKAPI_CALL vk_realloc(void *user, void *original, usize size, usize align,
                                           VkSystemAllocationScope scope);

        static void VKAPI_CALL vk_free(void *user, void *memory);

        static void VKAPI_CALL vk_internal_alloc(void *user, usize size,
                                                 VkInternalAllocationType type,
                                                 VkSystemAllocationScope  scope);

        static void VKAPI_CALL vk_internal_free(void *user, usize size,
                                                VkInternalAllocationType type,
                                                VkSystemAllocationScope  scope);

      private:
        TlsfAllocator        *m_tlsf = nullptr;
        VkAllocationCallbacks m_callbacks = {};
    };

} // namespace mantle
