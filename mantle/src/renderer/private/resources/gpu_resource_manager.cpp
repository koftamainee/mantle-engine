#include "renderer/gpu_resource_manager.h"

#include "core/assert.h"
#include "core/memory/persistent_allocator.h"
#include "gpu_resource_manager_internal.h"
#include "vulkan/vkassert.h"
#include "vulkan/vulkan_backend.h"
#include "vulkan/vulkan_utils.h"

namespace mantle {
    GPUResourceManager::~GPUResourceManager() { destroy(); }

    ShaderHandle
    GPUResourceManager::create_shader(std::span<const u32> spir_v) {
        VkShaderModuleCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = spir_v.size_bytes(),
            .pCode = spir_v.data(),
        };
        VkShaderModule shader_module = VK_NULL_HANDLE;
        vk_verify(vkCreateShaderModule(
            m_impl->backend->m_device.get_device(), &info,
            m_impl->backend->m_vk_allocator.vk_allocator(), &shader_module));

        u32 index = 0;
        u32 generation = 0;
        u32 free_list_size = m_impl->shaders_free_list.size();
        if (!m_impl->shaders_free_list.empty()) {
            index = m_impl->shaders_free_list[free_list_size - 1];
            check(index < m_impl->shaders.size());
            m_impl->shaders_free_list.pop_back();
            generation = m_impl->shaders[index].generation;
            m_impl->shaders[index].resource = {
                .shader = shader_module,
            };
        } else {
            index = m_impl->shaders.size();
            generation = 0;
            m_impl->shaders.push_back({{shader_module}, generation});
        }

        return {
            .index = index,
            .generation = generation,
        };
    }

    void GPUResourceManager::destroy_shader(ShaderHandle handle) {
        check(m_is_initialized);
        check(handle.index < m_impl->shaders.size());

        auto &shader = m_impl->shaders[handle.index];
        fatal(handle.generation != shader.generation, "Invalid shader handle");

        shader.generation++;
        m_impl->shaders_free_list.push_back(handle.index);

        auto del = [shader_module = shader.resource.shader, this]() {
            if (shader_module != VK_NULL_HANDLE) {
                vkDestroyShaderModule(
                    m_impl->backend->m_device.get_device(), shader_module,
                    m_impl->backend->m_vk_allocator.vk_allocator());
            }
        };
        shader.resource.shader = VK_NULL_HANDLE;

        m_impl->deletion_queues[m_impl->current_frame].push_fn(del);
    }

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
        if (!m_impl->buffers_free_list.empty()) {
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
            if (buf != VK_NULL_HANDLE) {
                m_impl->gpu_allocator.destroy_buffer(buf, alloc);
            }
        };
        buffer.resource.buffer = VK_NULL_HANDLE;
        buffer.resource.allocation = VK_NULL_HANDLE;

        m_impl->deletion_queues[m_impl->current_frame].push_fn(del);
    }

    ImageHandle GPUResourceManager::create_image(const ImageDesc &desc) {
        check(m_is_initialized);
        check(desc.width > 0);
        check(desc.height > 0);
        check(desc.mip_levels > 0);
        check(desc.array_layers > 0);
        checkf(!(desc.depth > 1 && desc.array_layers > 1),
               "3D array textures are not supported");

        VkImage image;
        VmaAllocation allocation;

        const u32 depth = desc.depth > 0 ? desc.depth : 1;

        VkImageCreateInfo image_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = (depth > 1) ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
            .format = to_vk(desc.format),
            .extent =
                VkExtent3D{
                    .width = desc.width,
                    .height = desc.height,
                    .depth = depth,
                },
            .mipLevels = desc.mip_levels,
            .arrayLayers = desc.array_layers,
            .samples = to_vk(desc.sample_count),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = to_vk(desc.usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        m_impl->gpu_allocator.create_image(
            image_create_info, VMA_MEMORY_USAGE_GPU_ONLY, &image, &allocation);


        VkImageViewCreateInfo view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = desc.depth > 1  ? VK_IMAGE_VIEW_TYPE_3D
                : desc.array_layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                        : VK_IMAGE_VIEW_TYPE_2D,
            .format = to_vk(desc.format),
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange =
                {
                    .aspectMask = to_vk_aspect(desc.format),
                    .baseMipLevel = 0,
                    .levelCount = desc.mip_levels,
                    .baseArrayLayer = 0,
                    .layerCount = desc.array_layers,
                },
        };

        VkImageView view = VK_NULL_HANDLE;
        if (desc.create_view) {
            vk_verify(vkCreateImageView(
                m_impl->backend->m_device.get_device(), &view_create_info,
                m_impl->backend->m_vk_allocator.vk_allocator(), &view));
        }
        u32 index = 0;
        u32 generation = 0;
        u32 free_list_size = m_impl->images_free_list.size();
        if (free_list_size > 0) {
            index = m_impl->images_free_list[free_list_size - 1];
            check(index < m_impl->images.size());
            m_impl->images_free_list.pop_back();
            generation = m_impl->images[index].generation;
            m_impl->images[index].resource = {
                .image = image,
                .allocation = allocation,
                .view = view,
            };
        } else {
            index = m_impl->images.size();
            generation = 0;
            m_impl->images.push_back(
                {{.image = image, .allocation = allocation, .view = view},
                 generation});
        }

        return {
            .index = index,
            .generation = generation,
        };
    }

    void GPUResourceManager::destroy_image(ImageHandle handle) {
        check(m_is_initialized);
        check(handle.index < m_impl->images.size());

        auto &image = m_impl->images[handle.index];
        fatal(handle.generation != image.generation, "Invalid image handle");

        image.generation++;
        m_impl->images_free_list.push_back(handle.index);

        auto del = [img = image.resource.image,
                    alloc = image.resource.allocation,
                    view = image.resource.view, this]() {
            if (alloc != VK_NULL_HANDLE) { // Case for swapchain images
                vkDestroyImageView(
                    m_impl->backend->m_device.get_device(), view,
                    m_impl->backend->m_vk_allocator.vk_allocator());
                m_impl->gpu_allocator.destroy_image(img, alloc);
            }
        };
        image.resource.image = VK_NULL_HANDLE;
        image.resource.view = VK_NULL_HANDLE;
        image.resource.allocation = VK_NULL_HANDLE;

        m_impl->deletion_queues[m_impl->current_frame].push_fn(del);
    }

    SamplerHandle GPUResourceManager::create_sampler(const SamplerDesc &desc) {
        check(m_is_initialized);

        VkSamplerCreateInfo sampler_create_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = to_vk(desc.mag_filter),
            .minFilter = to_vk(desc.min_filter),
            .mipmapMode = to_vk_mip(desc.mip_filter),
            .addressModeU = to_vk(desc.address_u),
            .addressModeV = to_vk(desc.address_v),
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = desc.max_anisotropy > 1.0f ? VK_TRUE : VK_FALSE,
            .maxAnisotropy = desc.max_anisotropy,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = desc.min_lod,
            .maxLod = desc.max_lod,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkSampler sampler;
        vk_verify(vkCreateSampler(
            m_impl->backend->m_device.get_device(), &sampler_create_info,
            m_impl->backend->m_vk_allocator.vk_allocator(), &sampler));

        u32 index = 0;
        u32 generation = 0;
        u32 free_list_size = m_impl->samplers_free_list.size();
        if (free_list_size > 0) {
            index = m_impl->samplers_free_list[free_list_size - 1];
            check(index < m_impl->samplers.size());
            m_impl->samplers_free_list.pop_back();
            generation = m_impl->samplers[index].generation;
            m_impl->samplers[index].resource = {
                .sampler = sampler,
            };
        } else {
            index = m_impl->samplers.size();
            generation = 0;
            m_impl->samplers.push_back({{.sampler = sampler}, generation});
        }

        return {
            .index = index,
            .generation = generation,
        };
    }

    void GPUResourceManager::destroy_sampler(SamplerHandle handle) {
        check(m_is_initialized);
        check(handle.index < m_impl->samplers.size());

        auto &sampler = m_impl->samplers[handle.index];
        fatal(handle.generation != sampler.generation,
              "Invalid sampler handle");

        sampler.generation++;
        m_impl->samplers_free_list.push_back(handle.index);

        auto del = [s = sampler.resource.sampler, this]() {
            if (s != VK_NULL_HANDLE) {
                vkDestroySampler(m_impl->backend->m_device.get_device(), s,
                                 m_impl->backend->m_vk_allocator.vk_allocator());
            }
        };
        sampler.resource.sampler = VK_NULL_HANDLE;

        m_impl->deletion_queues[m_impl->current_frame].push_fn(del);
    }

    u32 GPUResourceManager::get_bindless_index(SamplerHandle sampler) {}

    void GPUResourceManager::import_swapchain_images(
        std::pmr::vector<ImageHandle> &out_images) {
        check(m_is_initialized);

        const auto &swapchain_images =
            m_impl->backend->m_swapchain.get_images();
        VkFormat format =
            m_impl->backend->m_swapchain.get_surface_format().format;
        const u32 count = static_cast<u32>(swapchain_images.size());

        out_images.resize(count);

        for (usize i = 0; i < count; i++) {
            VkImage image = swapchain_images[i].image;
            VkImageView view = swapchain_images[i].view;

            u32 index;
            u32 generation;

            if (!m_impl->images_free_list.empty()) {
                index = m_impl->images_free_list.back();
                m_impl->images_free_list.pop_back();

                generation = m_impl->images[index].generation;
            } else {
                index = static_cast<u32>(m_impl->images.size());
                generation = 0;

                m_impl->images.push_back({{}, generation});
            }

            const auto &extent = m_impl->backend->m_swapchain.get_extent();

            m_impl->images[index].resource = {
                .image = image,
                .allocation = VK_NULL_HANDLE,
                .view = view,
                .desc =
                    {
                        .width = extent.width,
                        .height = extent.height,
                        .depth = 1,
                        .mip_levels = 1,
                        .format = from_vk(format),
                        .usage = ImageUsage::Color, // also usage::present, but
                                                    // its internal
                    },
            };

            out_images[i] = {.index = index, .generation = generation};
        }
    }

    void GPUResourceManager::release_swapchain_images(
        std::pmr::vector<ImageHandle> &images) {
        for (auto &image : images) {
            destroy_image(image);
        }
        images
            .clear(); // removes images, but capacity remains, so no memory leak
    }

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

        m_impl->images =
            std::pmr::vector<Slot<ImageResource>>(&m_impl->resource);
        m_impl->images_free_list = std::pmr::vector<u32>(&m_impl->resource);

        m_impl->samplers =
            std::pmr::vector<Slot<SamplerResource>>(&m_impl->resource);
        m_impl->samplers_free_list = std::pmr::vector<u32>(&m_impl->resource);

        m_impl->shaders =
            std::pmr::vector<Slot<ShaderResource>>(&m_impl->resource);
        m_impl->shaders_free_list = std::pmr::vector<u32>(&m_impl->resource);

        m_impl->gpu_allocator.init(backend->m_device.get_physical_device(),
                                   backend->m_device.get_device(),
                                   backend->m_context.get_instance(),
                                   backend->m_vk_allocator.vk_allocator());

        for (auto &queue : m_impl->deletion_queues) {
            queue.init(m_impl->backend->m_heap);
        }

        m_is_initialized = true;
        spdlog::info("GPU resource manager is initialized");
    }

void GPUResourceManager::destroy() {
    if (m_is_initialized) {
        m_impl->backend->wait_idle();

        for (u32 i = 0; i < m_impl->shaders.size(); i++) {
            if (m_impl->shaders[i].resource.shader != VK_NULL_HANDLE) {
                destroy_shader({i, m_impl->shaders[i].generation});
            }
        }

        for (u32 i = 0; i < m_impl->buffers.size(); i++) {
            if (m_impl->buffers[i].resource.buffer != VK_NULL_HANDLE) {
                destroy_buffer({i, m_impl->buffers[i].generation});
            }
        }

        for (u32 i = 0; i < m_impl->images.size(); ++i) {
            if (m_impl->images[i].resource.allocation != VK_NULL_HANDLE) {
                destroy_image({i, m_impl->images[i].generation});
            }
        }

        for (u32 i = 0; i < m_impl->samplers.size(); ++i) {
            if (m_impl->samplers[i].resource.sampler != VK_NULL_HANDLE) {
                destroy_sampler({i, m_impl->samplers[i].generation});
            }
        }

        for (auto& queue : m_impl->deletion_queues) {
            queue.flush();
        }

        m_impl->gpu_allocator.destroy();

        m_is_initialized = false;
        spdlog::info("GPU resource manager is destroyed");
    }
}

    u32 GPUResourceManager::get_bindless_index(ImageHandle image) {}

    u32 GPUResourceManager::get_bindless_index(BufferHandle buffer) {}

    ImageResource &GPUResourceManager::Impl::get_image(ImageHandle handle) {
        check(handle.index < images.size());
        auto &image = images[handle.index];
        if (image.generation != handle.generation) {
            fatal(true, "Attempting to get invalid buffer");
        }

        return image.resource;
    }

    BufferResource &GPUResourceManager::Impl::get_buffer(BufferHandle handle) {
        check(handle.index < buffers.size());
        auto &buffer = buffers[handle.index];
        if (buffer.generation != handle.generation) {
            fatal(true, "Attempting to get invalid buffer");
        }

        return buffer.resource;
    }
    SamplerResource &
    GPUResourceManager::Impl::get_sampler(SamplerHandle handle) {
        check(handle.index < samplers.size());
        auto &sampler = samplers[handle.index];
        if (sampler.generation != handle.generation) {
            fatal(true, "Attempting to get invalid sampler");
        }

        return sampler.resource;
    }

    ShaderResource &GPUResourceManager::Impl::get_shader(ShaderHandle handle) {
        check(handle.index < shaders.size());
        auto &shader = shaders[handle.index];
        if (shader.generation != handle.generation) {
            fatal(true, "Attempting to get invalid shader");
        }

        return shader.resource;
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
