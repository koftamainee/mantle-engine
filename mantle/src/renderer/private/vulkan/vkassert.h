#pragma once

namespace mantle {
#define MANTLE_VK_VERIFY(vk_call)                                                     \
    do {                                                                       \
        const VkResult vk_result = (vk_call);                                  \
        MANTLE_FATAL(vk_result != VK_SUCCESS, "VkResult != VK_SUCCESS");              \
    }                                                                          \
    while (0)


} // namespace mantle
