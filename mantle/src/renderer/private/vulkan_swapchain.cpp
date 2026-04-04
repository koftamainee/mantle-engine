#include "vulkan_swapchain.h"

#include <algorithm>
#include <array>
#include <vulkan/vulkan.h>

#include "vulkan_types.h"
#include <spdlog/spdlog.h>

#include "vkassert.h"
#include "core/assert.h"

namespace mantle {

VulkanSwapchain::~VulkanSwapchain() {
  destroy();
}

void VulkanSwapchain::init(VkDevice device,
                           VkSurfaceKHR surface,
                           const SwapchainSupportDetails& support_details,
                           const QueueFamilyIndices& indices,
                           uint32_t width,
                           uint32_t height) {
  check(!m_is_initialized);
  check(device != VK_NULL_HANDLE);
  this->m_device = device;

  m_surface_format = pick_surface_format(support_details.formats);
  m_extent = pick_extent(support_details.capabilities, width, height);
  m_present_mode = pick_present_mode(support_details.present_modes);

  uint32_t image_count = support_details.capabilities.minImageCount + 1;
  if (support_details.capabilities.maxImageCount > 0 &&
      image_count > support_details.capabilities.maxImageCount) {
    image_count = support_details.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .flags = VkSwapchainCreateFlagsKHR(),
    .surface = surface,
    .minImageCount = image_count,
    .imageFormat = m_surface_format.format,
    .imageColorSpace = m_surface_format.colorSpace,
    .imageExtent = m_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = support_details.capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = m_present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE,
  };

  const std::array indices_array = {
    indices.graphics_family,
    indices.present_family
  };

  if (indices_array[0] != indices_array[1]) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = indices_array.data();
  }

  vk_verify(vkCreateSwapchainKHR(device, &create_info, nullptr, &m_swapchain));

  vk_verify(vkGetSwapchainImagesKHR(device, m_swapchain, &image_count, nullptr));
  m_images.resize(image_count);
  vk_verify(vkGetSwapchainImagesKHR(device, m_swapchain, &image_count, m_images.data()));

  m_is_initialized = true;
  spdlog::info("Swapchain {}x{} created", m_extent.width, m_extent.height);
}

void VulkanSwapchain::destroy() {
  if (m_is_initialized) {
    check(m_device != VK_NULL_HANDLE);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    m_device = VK_NULL_HANDLE;
    m_is_initialized = false;
    spdlog::info("Swapchain destroyed");
  }
}

VkSurfaceFormatKHR VulkanSwapchain::pick_surface_format(const std::vector<VkSurfaceFormatKHR>& formats) {
  for (const auto& format : formats) {
    if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  fatal(true, "Unsupported surface format");
}

VkExtent2D VulkanSwapchain::pick_extent(const VkSurfaceCapabilitiesKHR& capabilities,
                                       uint32_t width,
                                       uint32_t height) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  }

  return {
    std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
    std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
  };
}

VkPresentModeKHR VulkanSwapchain::pick_present_mode(const std::vector<VkPresentModeKHR>& present_modes) {
  for (const auto& mode : present_modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  fatal(true, "Unsupported present mode");
}

} // namespace mantle