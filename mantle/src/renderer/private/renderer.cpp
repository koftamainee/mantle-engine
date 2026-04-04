#include <renderer/renderer.h>
#include <vulkan_graphics_context.h>
#include <vulkan_device.h>
#include <vulkan_swapchain.h>
#include <core/assert.h>
#include <spdlog/spdlog.h>

#include "window/window.h"

namespace mantle {

  Renderer::Renderer() = default;
  Renderer::~Renderer() { destroy(); }

  void Renderer::init(const Window& window) {
    check(!m_is_initialized);

    m_graphics_context = std::make_unique<VulkanGraphicsContext>();
    m_device = std::make_unique<VulkanDevice>();
    m_swapchain = std::make_unique<VulkanSwapchain>();

    m_graphics_context->init(window.get_native_window());
    VkSurfaceKHR surface = m_graphics_context->get_surface();

    m_device->init(m_graphics_context->get_instance(), surface);

    auto [width, height] = window.get_size();

    m_swapchain->init(
        m_device->get_device(),
        surface,
        m_device->get_swapchain_support_details(surface),
        m_device->get_queue_families(),
        width,
        height
    );

    m_is_initialized = true;
    spdlog::info("Renderer Initialized");
  }

  void Renderer::destroy() {
    if (m_is_initialized) {
      m_swapchain->destroy();
      m_device->destroy();
      m_graphics_context->destroy();

      m_swapchain.reset();
      m_device.reset();
      m_graphics_context.reset();

      spdlog::info("Renderer Destroyed");
      m_is_initialized = false;
    }
  }

} // namespace VkEngine