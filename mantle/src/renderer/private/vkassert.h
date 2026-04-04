#pragma once

namespace mantle {
#define vk_verify(vk_call)                                \
do {                                                      \
const VkResult vk_result = (vk_call);                     \
fatal(vk_result != VK_SUCCESS, "VkResult != VK_SUCCESS"); \
} while (0)
} // namespace mantle
