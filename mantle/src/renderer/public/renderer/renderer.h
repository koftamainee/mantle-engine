#pragma once
#include <memory>

namespace mantle {

  class VulkanSwapchain;
  class VulkanDevice;
  class VulkanGraphicsContext;
  class Window;

  class Renderer final {
  public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(const Renderer&&) noexcept = delete;
    Renderer& operator=(const Renderer&&) noexcept = delete;

    void init(const Window& window);
    void destroy();

  private:
    bool m_is_initialized = false;

    std::unique_ptr<VulkanGraphicsContext> m_graphics_context;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<VulkanSwapchain> m_swapchain;
  };

} // namespace VkEngine