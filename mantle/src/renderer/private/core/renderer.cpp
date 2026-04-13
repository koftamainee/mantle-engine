#include "renderer/renderer.h"
#include <spdlog/spdlog.h>
#include "core/memory/persistent_allocator.h"
#include "core/memory/pmr/persistent_resource.h"
#include "vulkan/command_recorder.h"
#include "vulkan/frame_scheduler.h"
#include "vulkan/vulkan_backend.h"

namespace mantle {

    struct Renderer::Impl final {
        VulkanBackend backend{};
        FrameScheduler frame_scheduler{};
        GPUResourceManager resource_manager{};
        ImageHandle backbuffer{};
        FrameContext current_frame{};

        PersistentResource persistent_resource{};
        std::pmr::vector<ImageHandle> swapchain_images{};
    };

    Renderer::~Renderer() { destroy(); }

    void Renderer::init(const Window &window, bool vsync, VirtualHeap *heap,
                        ArenaAllocator *scratch_arena) {
        check(!m_is_initialized);

        PersistentAllocator alloc;
        alloc.init(heap);
        m_impl = alloc.emplace<Impl>();

        m_impl->persistent_resource = PersistentResource(heap);
        m_impl->swapchain_images =
            std::pmr::vector<ImageHandle>(&m_impl->persistent_resource);

        m_impl->backend.init(window, vsync, heap, scratch_arena);
        m_impl->frame_scheduler.init(&m_impl->backend, 3);
        m_impl->resource_manager.init(&m_impl->backend);

        m_impl->resource_manager.import_swapchain_images(
            m_impl->backend.get_swapchain_info(), m_impl->swapchain_images);

        m_is_initialized = true;
        spdlog::info("Renderer is initialized");
    }

    void Renderer::destroy() {
        if (!m_is_initialized) {
            return;
        }
        if (m_impl == nullptr) {
            return;
        }
        m_impl->backend.wait_idle();
        m_impl->resource_manager.release_swapchain_images(
            m_impl->swapchain_images);
        m_impl->resource_manager.destroy();
        m_impl->frame_scheduler.destroy();
        m_impl->backend.destroy();
        m_impl = nullptr;
        m_is_initialized = false;
        spdlog::info("Renderer is destroyed");
    }

    Renderer::Result Renderer::begin_frame() {
        check(m_is_initialized);

        FrameResult result =
            m_impl->frame_scheduler.begin_frame(m_impl->current_frame);

        if (result == FrameResult::NeedsResize) {
            return Result::FrameNeedsResize;
        }

        m_impl->backbuffer =
            m_impl->swapchain_images[m_impl->current_frame.image_index];

        return Result::Ok;
    }

    void Renderer::execute(const CompiledRenderGraph &graph) {
        check(m_is_initialized);
    }

    Renderer::Result Renderer::end_frame() {
        check(m_is_initialized);

        FrameResult result =
            m_impl->frame_scheduler.end_frame(m_impl->current_frame);

        if (result == FrameResult::NeedsResize) {
            return Result::FrameNeedsResize;
        }

        return Result::Ok;
    }

    void Renderer::resize(u32 width, u32 height) {
        check(m_is_initialized);
        m_impl->backend.wait_idle();

        m_impl->resource_manager.release_swapchain_images(
            m_impl->swapchain_images);

        m_impl->backend.rebuild_swapchain(width, height);

        m_impl->frame_scheduler.on_swapchain_rebuilt(
            m_impl->backend.get_swapchain_info().image_count);

        m_impl->resource_manager.import_swapchain_images(
            m_impl->backend.get_swapchain_info(), m_impl->swapchain_images);
    }

    ImageHandle Renderer::backbuffer() const {
        check(m_is_initialized);
        return m_impl->backbuffer;
    }

    GPUResourceManager &Renderer::resource_manager() {
        check(m_is_initialized);
        return m_impl->resource_manager;
    }

} // namespace mantle
