#include "renderer/gpu_resource_manager.h"

#include "core/assert.h"
#include "core/memory/persistent_allocator.h"
#include "gpu_resource_manager_internal.h"
#include "vulkan/vulkan_backend.h"
#include "vulkan/vulkan_utils.h"

namespace mantle {
    GPUResourceManager::~GPUResourceManager() { destroy(); }

    ShaderHandle
    GPUResourceManager::create_shader(std::span<const u32> spir_v) {
        return {};
    }

    void GPUResourceManager::destroy_shader(ShaderHandle shader) {}

    GraphicsPipelineHandle GPUResourceManager::create_graphics_pipeline(
        const GraphicsPipelineDesc &desc) {
        return {};
    }

    ComputePipelineHandle GPUResourceManager::create_compute_pipeline(
        const ComputePipelineDesc &desc) {
        return {};
    }

    void GPUResourceManager::destroy_graphics_pipeline(
        GraphicsPipelineHandle pipeline) {}

    void GPUResourceManager::destroy_compute_pipeline(
        ComputePipelineHandle pipeline) {}

    BufferHandle GPUResourceManager::create_buffer(const BufferDesc &desc,
                                                   bool map) {
        check(m_is_initialized);
        VkBuffer buffer;
        VmaAllocation allocation;
        void *memory = nullptr;

        checkf(!(map && desc.memory == MemoryType::Gpu),
               "Gpu only buffer can not be used for mapping");


        if (map) {
            m_impl->gpu_allocator.create_buffer(desc.size, to_vk(desc.usage),
                                                to_vma(desc.memory), &buffer,
                                                &allocation, &memory);
        } else {
            m_impl->gpu_allocator.create_buffer(desc.size, to_vk(desc.usage),
                                                to_vma(desc.memory), &buffer,
                                                &allocation);
        }

        u32 index = 0;
        u32 generation = 0;
        u32 free_list_size = m_impl->buffers_free_list.size();
        if (m_impl->buffers_free_list.size() > 0) {
            index = m_impl->buffers_free_list[free_list_size - 1];
            check(index < m_impl->buffers.size());
            m_impl->buffers_free_list.pop_back();
            generation = m_impl->buffers[index].generation;
            m_impl->buffers[index].resource = {
                .buffer = buffer,
                .allocation = allocation,
                .mapped = memory,
            };
        } else {
            index = m_impl->buffers.size();
            generation = 0;
            m_impl->buffers.push_back(
                {{.buffer = buffer, .allocation = allocation, .mapped = memory},
                 generation});
        }

        return {
            .index = index,
            .generation = generation,
        };
    }

    void GPUResourceManager::update_buffer(BufferHandle handle,
                                           const void *data, usize size,
                                           usize offset) {
        check(m_is_initialized);
        check(handle.index < m_impl->buffers.size());

        auto &buffer = m_impl->buffers[handle.index];
        fatal(handle.generation != buffer.generation, "Invalid buffer handle");
        checkf(buffer.resource.mapped != nullptr,
               "Attempting to write in device local memory buffer");

        memcpy(static_cast<u8 *>(buffer.resource.mapped) + offset, data, size);
    }

    void GPUResourceManager::destroy_buffer(BufferHandle handle) {
        check(m_is_initialized);
        check(handle.index < m_impl->buffers.size());

        auto &buffer = m_impl->buffers[handle.index];
        fatal(handle.generation != buffer.generation, "Invalid buffer handle");

        buffer.generation++;
        m_impl->buffers_free_list.push_back(handle.index);

        auto del = [buf = buffer.resource.buffer,
                    alloc = buffer.resource.allocation, this]() {
            m_impl->gpu_allocator.destroy_buffer(buf, alloc);
        };

        m_impl->deletion_queues[m_impl->current_frame].push(del);
    }

    ImageHandle GPUResourceManager::create_image(const ImageDesc &desc) {
        return {};
    }

    void GPUResourceManager::destroy_image(ImageHandle image) {}

    SamplerHandle GPUResourceManager::create_sampler(const SamplerDesc &desc) {}

    void GPUResourceManager::destroy_sampler(SamplerHandle sampler) {}

    u32 GPUResourceManager::get_bindless_index(SamplerHandle sampler) {}

    void GPUResourceManager::import_swapchain_images(
        const SwapchainInfo &swapchain_info,
        std::pmr::vector<ImageHandle> &out_images) {
        out_images.resize(swapchain_info.image_count);
    }

    void GPUResourceManager::release_swapchain_images(
        std::pmr::vector<ImageHandle> &images) {}

    void GPUResourceManager::init(VulkanBackend *backend) {
        check(!m_is_initialized);
        check(backend != nullptr);

        PersistentAllocator alloc;
        alloc.init(backend->m_heap);
        m_impl = alloc.emplace<Impl>();
        check(m_impl != nullptr);

        m_impl->backend = backend;
        m_impl->resource = PersistentResource(backend->m_heap);

        m_impl->buffers =
            std::pmr::vector<Slot<BufferResource>>(&m_impl->resource);
        m_impl->buffers_free_list = std::pmr::vector<u32>(&m_impl->resource);


        m_impl->gpu_allocator.init(backend->m_device.get_physical_device(),
                                   backend->m_device.get_device(),
                                   backend->m_context.get_instance(),
                                   backend->m_vk_allocator.vk_allocator());


        m_is_initialized = true;
        spdlog::info("GPU resource manager is initialized");
    }

    void GPUResourceManager::destroy() {
        if (m_is_initialized) {
            m_impl->gpu_allocator.destroy();

            m_is_initialized = false;
            spdlog::info("GPU resource manager is destroyed");
        }
    }

    u32 GPUResourceManager::get_bindless_index(ImageHandle image) {}

    u32 GPUResourceManager::get_bindless_index(BufferHandle buffer) {}

    VkImage GPUResourceManager::Impl::get_vk_image(ImageHandle handle) const {}

    VkImageView
    GPUResourceManager::Impl::get_vk_image_view(ImageHandle handle) const {}

    VkBuffer
    GPUResourceManager::Impl::get_vk_buffer(BufferHandle handle) const {
        check(handle.index < buffers.size());
        auto &buffer = buffers[handle.index];
        if (buffer.generation != handle.generation) {
            spdlog::error("Attempting to get invalid buffer");
            return VK_NULL_HANDLE;
        }

        return buffer.resource.buffer;
    }

    VkPipeline GPUResourceManager::Impl::get_vk_pipeline(
        GraphicsPipelineHandle handle) const {}

    VkPipeline GPUResourceManager::Impl::get_vk_pipeline(
        ComputePipelineHandle handle) const {}

    VkPipelineLayout GPUResourceManager::Impl::get_vk_pipeline_layout(
        GraphicsPipelineHandle handle) const {}

    VkPipelineLayout GPUResourceManager::Impl::get_vk_pipeline_layout(
        ComputePipelineHandle handle) const {}

    void GPUResourceManager::Impl::next_frame() {
        current_frame = (current_frame + 1) % frame_lag;

        deletion_queues[current_frame].flush();
    }

} // namespace mantle
