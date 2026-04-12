#include <core/assert.h>
#include <renderer/renderer.h>
#include <spdlog/spdlog.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "vulkan/vulkan_context.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_swapchain.h"

#include "core/memory/persistent_allocator.h"
#include "renderer_impl.h"

#include "vulkan/vkassert.h"
#include "window/window.h"

namespace mantle {

    Renderer::Renderer() = default;
    Renderer::~Renderer() { destroy(); }

    void Renderer::init(const Window &window, VirtualHeap *heap,
                        ArenaAllocator *scratch_arena) {
        check(!m_is_initialized);
        check(heap != nullptr);

        PersistentAllocator alloc;
        alloc.init(heap);

        m_impl = alloc.emplace<Impl>();
        m_impl->init(window, heap, scratch_arena);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window.get_native_window(), true);
        /* TODO:
         * 1. Add vulkan descriptor pools, sets and set layouts handling
         * 2. Initialize vulkan backend for imgui
         * 3. Probably carry this out into separate ImGui only class
         * 4. Renderer and m_impl are becoming too big, refactor them later
         */

        m_is_initialized = true;
        spdlog::info("Renderer Initialized");
    }

    void Renderer::destroy() {
        if (m_is_initialized) {

            m_impl->destroy();
            m_impl->~Impl();

            spdlog::info("Renderer Destroyed");
            m_is_initialized = false;
        }
    }


    Renderer::Result Renderer::begin_frame() const {
        check(m_is_initialized);

        if (m_impl->swapchain_dirty) {
            return Result::FrameNeedsResize;
        }

        FrameData &frame = m_impl->get_current_frame();
        VkDevice device = m_impl->device.get_device();

        vk_verify(
            vkWaitForFences(device, 1, &frame.in_flight, VK_TRUE, UINT64_MAX));
        vk_verify(vkResetFences(device, 1, &frame.in_flight));

        VkSemaphore acquire_sem =
            m_impl->acquire_semaphores[m_impl->acquire_index];

        VkResult result = vkAcquireNextImageKHR(
            device, m_impl->swapchain.get_swapchain(), UINT64_MAX, acquire_sem,
            VK_NULL_HANDLE, &m_impl->image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            m_impl->swapchain_dirty = true;
            return Result::FrameNeedsResize;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            vk_verify(result);
        }

        vk_verify(vkResetCommandBuffer(frame.cmd, 0));

        VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        vk_verify(vkBeginCommandBuffer(frame.cmd, &begin_info));

        return Result::Ok;
    }

    Renderer::Result Renderer::end_frame() const {
        auto &frame = m_impl->get_current_frame();

        VkSemaphore acquire_sem =
            m_impl->acquire_semaphores[m_impl->acquire_index];
        VkSemaphore render_sem = m_impl->render_semaphores[m_impl->image_index];

        vk_verify(vkEndCommandBuffer(frame.cmd));

        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &acquire_sem,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &frame.cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &render_sem,
        };

        vk_verify(vkQueueSubmit(m_impl->device.get_graphics_queue(), 1, &submit,
                                frame.in_flight));

        VkSwapchainKHR swapchain = m_impl->swapchain.get_swapchain();

        VkPresentInfoKHR present = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render_sem,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &m_impl->image_index,
        };

        VkResult result =
            vkQueuePresentKHR(m_impl->device.get_present_queue(), &present);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            m_impl->swapchain_dirty = true;
        } else {
            vk_verify(result);
        }

        m_impl->current_frame =
            (m_impl->current_frame + 1) % Impl::frames_in_flight;
        m_impl->acquire_index = (m_impl->acquire_index + 1) %
            static_cast<u32>(m_impl->acquire_semaphores.size());

        if (m_impl->swapchain_dirty) {
            return Result::FrameNeedsResize;
        }
        return Result::Ok;
    }

    GPUResourceManager &Renderer::resource_manager() {} // TODO

    ImageHandle Renderer::current_backbuffer() const {} // TODO

    void Renderer::execute(const CompiledRenderGraph &render_graph) {} // TODO


    void Renderer::resize(u32 width, u32 height) const {
        check(m_is_initialized);
        VkDevice device = m_impl->device.get_device();
        VkSurfaceKHR surface = m_impl->graphics_context.get_surface();

        vkDeviceWaitIdle(device);

        m_impl->destroy_depth_image();

        u32 old_count = static_cast<u32>(m_impl->acquire_semaphores.size());

        m_impl->swapchain.destroy();
        m_impl->swapchain.init(
            device, surface,
            m_impl->device.get_swapchain_support_details(surface),
            m_impl->device.get_queue_families(), width, height,
            m_impl->vulkan_cpu_allocator.vk_allocator());

        m_impl->create_depth_image(width, height);

        // NOTE: I don't sure if this needed
        // This code handles case when image count in swapchain changes between
        // recreations This probably not gonna happen ever, but it useful to
        // have
        u32 new_count = static_cast<u32>(m_impl->swapchain.get_images().size());
        if (new_count != old_count) {
            spdlog::warn("POTENTIAL MEMORY LEAK. using virtual heap memory "
                         "resource to recreate objects");
            for (auto &sem : m_impl->acquire_semaphores) {
                vkDestroySemaphore(device, sem, nullptr);
            }
            for (auto &sem : m_impl->render_semaphores) {
                vkDestroySemaphore(device, sem, nullptr);
            }

            m_impl->acquire_semaphores.resize(new_count);
            m_impl->render_semaphores.resize(new_count);

            VkSemaphoreCreateInfo sem_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
            for (auto &sem : m_impl->acquire_semaphores) {
                vk_verify(vkCreateSemaphore(device, &sem_info, nullptr, &sem));
            }
            for (auto &sem : m_impl->render_semaphores) {
                vk_verify(vkCreateSemaphore(device, &sem_info, nullptr, &sem));
            }

            m_impl->acquire_index = 0;
        }

        m_impl->swapchain_dirty = false;
    }

} // namespace mantle
