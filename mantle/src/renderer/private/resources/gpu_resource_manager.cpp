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
        check(m_is_initialized);

        std::array<VkPipelineShaderStageCreateInfo, 5> stages{};
        u32 stage_count = 0;
        u32 stage_mask = 0;

        for (const auto &module : desc.shaders) {
            u32 stage_bit = static_cast<u32>(module.stage);
            checkf(!(stage_mask & (1u << stage_bit)),
                   "Duplicate shader stage in graphics pipeline: each stage "
                   "must appear at most once");
            stage_mask |= (1u << stage_bit);

            stages[stage_count++] = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage =
                    static_cast<VkShaderStageFlagBits>(to_vk(module.stage)),
                .module = m_impl->get_shader(module.shader).shader,
                .pName = module.entry_point.data(),
            };
        }
        checkf((stage_mask & (1u << static_cast<u32>(ShaderStage::Vertex))) !=
                   0,
               "Graphics pipeline must have a vertex shader");
        checkf((stage_mask & (1u << static_cast<u32>(ShaderStage::Fragment))) !=
                   0,
               "Graphics pipeline must have a fragment shader");

        std::array<VkVertexInputBindingDescription, 16> vk_bindings{};
        std::array<VkVertexInputAttributeDescription, 32> vk_attributes{};

        for (u32 i = 0; i < desc.vertex_input.bindings.size(); i++) {
            const auto &b = desc.vertex_input.bindings[i];
            vk_bindings[i] = {
                .binding = b.binding,
                .stride = b.stride,
                .inputRate = b.per_instance ? VK_VERTEX_INPUT_RATE_INSTANCE
                                            : VK_VERTEX_INPUT_RATE_VERTEX,
            };
        }
        for (u32 i = 0; i < desc.vertex_input.attributes.size(); i++) {
            const auto &a = desc.vertex_input.attributes[i];
            vk_attributes[i] = {
                .location = a.location,
                .binding = a.binding,
                .format = to_vk(a.format),
                .offset = a.offset,
            };
        }

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount =
                static_cast<u32>(desc.vertex_input.bindings.size()),
            .pVertexBindingDescriptions = vk_bindings.data(),
            .vertexAttributeDescriptionCount =
                static_cast<u32>(desc.vertex_input.attributes.size()),
            .pVertexAttributeDescriptions = vk_attributes.data(),
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = to_vk(desc.input_assembly.topology),
            .primitiveRestartEnable =
                desc.input_assembly.primitive_restart_enable,
        };

        VkPipelineTessellationStateCreateInfo tessellation_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .patchControlPoints = desc.tessellation.patch_control_points,
        };
        VkPipelineTessellationStateCreateInfo *p_tessellation = nullptr;
        if (stage_mask &
            ((1u << static_cast<u32>(ShaderStage::TessellationControl)) |
             (1u << static_cast<u32>(ShaderStage::TessellationEvaluation)))) {
            p_tessellation = &tessellation_info;
        }

        VkPipelineViewportStateCreateInfo viewport_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        VkPipelineRasterizationStateCreateInfo rasterization_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = desc.rasterization.depth_clamp_enable,
            .rasterizerDiscardEnable =
                desc.rasterization.rasterizer_discard_enable,
            .polygonMode = to_vk(desc.rasterization.polygon_mode),
            .cullMode = to_vk(desc.rasterization.cull_mode),
            .frontFace = to_vk(desc.rasterization.front_face),
            .depthBiasEnable = desc.rasterization.depth_bias_enable,
            .depthBiasConstantFactor =
                desc.rasterization.depth_bias_constant_factor,
            .depthBiasClamp = desc.rasterization.depth_bias_clamp,
            .depthBiasSlopeFactor = desc.rasterization.depth_bias_slope_factor,
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisample_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples =
                to_vk(desc.multisample.rasterization_samples),
            .sampleShadingEnable = desc.multisample.sample_shading_enable,
            .minSampleShading = desc.multisample.min_sample_shading,
            .alphaToCoverageEnable = desc.multisample.alpha_to_coverage_enable,
            .alphaToOneEnable = desc.multisample.alpha_to_one_enable,
        };

        VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = desc.depth_stencil.depth_test_enable,
            .depthWriteEnable = desc.depth_stencil.depth_write_enable,
            .depthCompareOp = to_vk(desc.depth_stencil.depth_compare_op),
            .depthBoundsTestEnable =
                desc.depth_stencil.depth_bounds_test_enable,
            .stencilTestEnable = desc.depth_stencil.stencil_test_enable,
            .front =
                {
                    .failOp = to_vk(desc.depth_stencil.front.fail_op),
                    .passOp = to_vk(desc.depth_stencil.front.pass_op),
                    .depthFailOp =
                        to_vk(desc.depth_stencil.front.depth_fail_op),
                    .compareOp = to_vk(desc.depth_stencil.front.compare_op),
                    .compareMask = desc.depth_stencil.front.compare_mask,
                    .writeMask = desc.depth_stencil.front.write_mask,
                    .reference = desc.depth_stencil.front.reference,
                },
            .back =
                {
                    .failOp = to_vk(desc.depth_stencil.back.fail_op),
                    .passOp = to_vk(desc.depth_stencil.back.pass_op),
                    .depthFailOp = to_vk(desc.depth_stencil.back.depth_fail_op),
                    .compareOp = to_vk(desc.depth_stencil.back.compare_op),
                    .compareMask = desc.depth_stencil.back.compare_mask,
                    .writeMask = desc.depth_stencil.back.write_mask,
                    .reference = desc.depth_stencil.back.reference,
                },
            .minDepthBounds = desc.depth_stencil.min_depth_bounds,
            .maxDepthBounds = desc.depth_stencil.max_depth_bounds,
        };

        std::array<VkPipelineColorBlendAttachmentState, 8> vk_attachments{};
        for (u32 i = 0; i < desc.color_blend.attachments.size(); i++) {
            const auto &a = desc.color_blend.attachments[i];
            vk_attachments[i] = {
                .blendEnable = a.blend_enable,
                .srcColorBlendFactor = to_vk(a.src_color_blend_factor),
                .dstColorBlendFactor = to_vk(a.dst_color_blend_factor),
                .colorBlendOp = to_vk(a.color_blend_op),
                .srcAlphaBlendFactor = to_vk(a.src_alpha_blend_factor),
                .dstAlphaBlendFactor = to_vk(a.dst_alpha_blend_factor),
                .alphaBlendOp = to_vk(a.alpha_blend_op),
                .colorWriteMask = to_vk_color_write_mask(a.color_write_mask),
            };
        }

        VkPipelineColorBlendStateCreateInfo color_blend_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = desc.color_blend.logic_op_enable,
            .logicOp = to_vk(desc.color_blend.logic_op),
            .attachmentCount =
                static_cast<u32>(desc.color_blend.attachments.size()),
            .pAttachments = vk_attachments.data(),
            .blendConstants =
                {
                    desc.color_blend.blend_constants[0],
                    desc.color_blend.blend_constants[1],
                    desc.color_blend.blend_constants[2],
                    desc.color_blend.blend_constants[3],
                },
        };

        std::array<VkDynamicState, 2> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo dynamic_state_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<u32>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data(),
        };

        std::array<VkPushConstantRange, 8> vk_push_constants{};
        for (u32 i = 0; i < desc.push_constants.size(); i++) {
            vk_push_constants[i] = {
                .stageFlags = to_vk(desc.push_constants[i].stage),
                .offset = desc.push_constants[i].offset,
                .size = desc.push_constants[i].size,
            };
        }

        VkPipelineLayoutCreateInfo layout_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pushConstantRangeCount =
                static_cast<u32>(desc.push_constants.size()),
            .pPushConstantRanges = vk_push_constants.data(),
        };

        VkPipelineLayout layout = VK_NULL_HANDLE;
        vk_verify(vkCreatePipelineLayout(
            m_impl->backend->m_device.get_device(), &layout_info,
            m_impl->backend->m_vk_allocator.vk_allocator(), &layout));

        std::array<VkFormat, 8> vk_color_formats{};
        for (u32 i = 0; i < desc.color_formats.size(); i++) {
            vk_color_formats[i] = to_vk(desc.color_formats[i]);
        }

        VkPipelineRenderingCreateInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = static_cast<u32>(desc.color_formats.size()),
            .pColorAttachmentFormats = vk_color_formats.data(),
            .depthAttachmentFormat = to_vk(desc.depth_format),
            .stencilAttachmentFormat = to_vk(desc.stencil_format),
        };

        VkGraphicsPipelineCreateInfo pipeline_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &rendering_info,
            .stageCount = stage_count,
            .pStages = stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly_info,
            .pTessellationState = p_tessellation,
            .pViewportState = &viewport_info,
            .pRasterizationState = &rasterization_info,
            .pMultisampleState = &multisample_info,
            .pDepthStencilState = &depth_stencil_info,
            .pColorBlendState = &color_blend_info,
            .pDynamicState = &dynamic_state_info,
            .layout = layout,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        vk_verify(vkCreateGraphicsPipelines(
            m_impl->backend->m_device.get_device(), VK_NULL_HANDLE, 1,
            &pipeline_info, m_impl->backend->m_vk_allocator.vk_allocator(),
            &pipeline));

        u32 index = 0;
        u32 generation = 0;
        if (!m_impl->graphics_pipelines_free_list.empty()) {
            index = m_impl->graphics_pipelines_free_list.back();
            check(index < m_impl->graphics_pipelines.size());
            m_impl->graphics_pipelines_free_list.pop_back();
            generation = m_impl->graphics_pipelines[index].generation;
            m_impl->graphics_pipelines[index].resource = {
                .pipeline = pipeline,
                .layout = layout,
                .desc = desc,
            };
        } else {
            index = m_impl->graphics_pipelines.size();
            generation = 0;
            m_impl->graphics_pipelines.push_back(
                {{.pipeline = pipeline, .layout = layout, .desc = desc},
                 generation});
        }

        return {
            .index = index,
            .generation = generation,
        };
    }


    ComputePipelineHandle GPUResourceManager::create_compute_pipeline(
        const ComputePipelineDesc &desc) {
        check(m_is_initialized);

        VkPushConstantRange push_constant_range = {
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .offset = 0,
            .size = desc.push_constant_size,
        };

        VkPipelineLayoutCreateInfo layout_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pushConstantRangeCount = desc.push_constant_size > 0 ? 1u : 0u,
            .pPushConstantRanges =
                desc.push_constant_size > 0 ? &push_constant_range : nullptr,
        };

        VkPipelineLayout layout = VK_NULL_HANDLE;
        vk_verify(vkCreatePipelineLayout(
            m_impl->backend->m_device.get_device(), &layout_info,
            m_impl->backend->m_vk_allocator.vk_allocator(), &layout));

        auto &shader = m_impl->get_shader(desc.shader.shader);

        VkPipelineShaderStageCreateInfo stage_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = shader.shader,
            .pName = desc.shader.entry_point.data(),
        };

        VkComputePipelineCreateInfo pipeline_info = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = stage_info,
            .layout = layout,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        vk_verify(vkCreateComputePipelines(
            m_impl->backend->m_device.get_device(), VK_NULL_HANDLE, 1,
            &pipeline_info, m_impl->backend->m_vk_allocator.vk_allocator(),
            &pipeline));


        u32 index = 0;
        u32 generation = 0;
        if (!m_impl->compute_pipelines_free_list.empty()) {
            index = m_impl->compute_pipelines_free_list.back();
            check(index < m_impl->compute_pipelines.size());
            m_impl->compute_pipelines_free_list.pop_back();
            generation = m_impl->compute_pipelines[index].generation;
            m_impl->compute_pipelines[index].resource = {
                .pipeline = pipeline,
                .layout = layout,
                .desc = desc,
            };
        } else {
            index = m_impl->compute_pipelines.size();
            generation = 0;
            m_impl->compute_pipelines.push_back(
                {{.pipeline = pipeline, .layout = layout, .desc = desc},
                 generation});
        }

        return {
            .index = index,
            .generation = generation,
        };
    }

    void GPUResourceManager::destroy_graphics_pipeline(
        GraphicsPipelineHandle handle) {
        check(m_is_initialized);
        check(handle.index < m_impl->graphics_pipelines.size());

        auto &pipeline = m_impl->graphics_pipelines[handle.index];
        fatal(handle.generation != pipeline.generation,
              "Invalid graphics pipeline handle");

        pipeline.generation++;
        m_impl->graphics_pipelines_free_list.push_back(handle.index);

        auto del = [vk_pipeline = pipeline.resource.pipeline,
                    vk_layout = pipeline.resource.layout, this]() {
            if (vk_pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(
                    m_impl->backend->m_device.get_device(), vk_pipeline,
                    m_impl->backend->m_vk_allocator.vk_allocator());
            }
            if (vk_layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(
                    m_impl->backend->m_device.get_device(), vk_layout,
                    m_impl->backend->m_vk_allocator.vk_allocator());
            }
        };
        pipeline.resource.pipeline = VK_NULL_HANDLE;
        pipeline.resource.layout = VK_NULL_HANDLE;

        m_impl->deletion_queues[m_impl->current_frame].push_fn(del);
    }


    void
    GPUResourceManager::destroy_compute_pipeline(ComputePipelineHandle handle) {
        check(m_is_initialized);
        check(handle.index < m_impl->compute_pipelines.size());

        auto &pipeline = m_impl->compute_pipelines[handle.index];
        fatal(handle.generation != pipeline.generation,
              "Invalid compute pipeline handle");

        pipeline.generation++;
        m_impl->compute_pipelines_free_list.push_back(handle.index);

        auto del = [vk_pipeline = pipeline.resource.pipeline,
                    vk_layout = pipeline.resource.layout, this]() {
            if (vk_pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(
                    m_impl->backend->m_device.get_device(), vk_pipeline,
                    m_impl->backend->m_vk_allocator.vk_allocator());
            }
            if (vk_layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(
                    m_impl->backend->m_device.get_device(), vk_layout,
                    m_impl->backend->m_vk_allocator.vk_allocator());
            }
        };
        pipeline.resource.pipeline = VK_NULL_HANDLE;
        pipeline.resource.layout = VK_NULL_HANDLE;

        m_impl->deletion_queues[m_impl->current_frame].push_fn(del);
    }

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
                vkDestroySampler(
                    m_impl->backend->m_device.get_device(), s,
                    m_impl->backend->m_vk_allocator.vk_allocator());
            }
        };
        sampler.resource.sampler = VK_NULL_HANDLE;

        m_impl->deletion_queues[m_impl->current_frame].push_fn(del);
    }

    u32 GPUResourceManager::get_bindless_index(SamplerHandle sampler) {} // TODO

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

        m_impl->graphics_pipelines =
            std::pmr::vector<Slot<GraphicsPipelineResource>>(&m_impl->resource);
        m_impl->graphics_pipelines_free_list =
            std::pmr::vector<u32>(&m_impl->resource);

        m_impl->compute_pipelines =
            std::pmr::vector<Slot<ComputePipelineResource>>(&m_impl->resource);
        m_impl->compute_pipelines_free_list =
            std::pmr::vector<u32>(&m_impl->resource);

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

            for (u32 i = 0; i < m_impl->compute_pipelines.size(); ++i) {
                if (m_impl->compute_pipelines[i].resource.pipeline !=
                    VK_NULL_HANDLE) {
                    destroy_compute_pipeline(
                        {i, m_impl->compute_pipelines[i].generation});
                    }
            }

            for (u32 i = 0; i < m_impl->graphics_pipelines.size(); ++i) {
                if (m_impl->graphics_pipelines[i].resource.pipeline !=
                    VK_NULL_HANDLE) {
                    destroy_graphics_pipeline(
                        {i, m_impl->graphics_pipelines[i].generation});
                    }
            }

            for (u32 i = 0; i < m_impl->shaders.size(); i++) {
                if (m_impl->shaders[i].resource.shader != VK_NULL_HANDLE) {
                    destroy_shader({i, m_impl->shaders[i].generation});
                }
            }

            for (u32 i = 0; i < m_impl->samplers.size(); ++i) {
                if (m_impl->samplers[i].resource.sampler != VK_NULL_HANDLE) {
                    destroy_sampler({i, m_impl->samplers[i].generation});
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


            for (auto &queue : m_impl->deletion_queues) {
                queue.flush();
            }

            m_impl->gpu_allocator.destroy();

            m_is_initialized = false;
            spdlog::info("GPU resource manager is destroyed");
        }
    }

    u32 GPUResourceManager::get_bindless_index(ImageHandle image) {} // TODO

    u32 GPUResourceManager::get_bindless_index(BufferHandle buffer) {} // TODO

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

    GraphicsPipelineResource &GPUResourceManager::Impl::get_graphics_pipeline(
        GraphicsPipelineHandle handle) {
        check(handle.index < graphics_pipelines.size());
        auto &pipeline = graphics_pipelines[handle.index];
        if (pipeline.generation != handle.generation) {
            fatal(true, "Attempting to get invalid graphics pipeline");
        }

        return pipeline.resource;
    }

    ComputePipelineResource &GPUResourceManager::Impl::get_compute_pipeline(
        ComputePipelineHandle handle) {
        check(handle.index < compute_pipelines.size());
        auto &pipeline = compute_pipelines[handle.index];
        if (pipeline.generation != handle.generation) {
            fatal(true, "Attempting to get invalid compute pipeline");
        }

        return pipeline.resource;
    }


    void GPUResourceManager::Impl::next_frame() {
        current_frame = (current_frame + 1) % frame_lag;

        deletion_queues[current_frame].flush();
    }

} // namespace mantle
