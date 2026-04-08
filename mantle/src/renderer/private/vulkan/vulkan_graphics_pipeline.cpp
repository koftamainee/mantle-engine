#include "vulkan_graphics_pipeline.h"

#include <array>
#include "core/assert.h"
#include "spdlog/spdlog.h"
#include "vkassert.h"

#include "mesh/vertex.h"

namespace mantle {

    static VkShaderModule create_shader_module(VkDevice device,
                                               std::span<u32> spv) {
        VkShaderModuleCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = spv.size() * sizeof(u32),
            .pCode = spv.data(),
        };
        VkShaderModule mod;
        vk_verify(vkCreateShaderModule(device, &info, nullptr, &mod));
        return mod;
    }

    static void destroy_shader_module(VkDevice device, VkShaderModule mod) {
        vkDestroyShaderModule(device, mod, nullptr);
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() { destroy(); }

    void VulkanGraphicsPipeline::init(VkDevice device, const Config &config,
                                      std::span<u32> spv) {
        check(!m_is_initialized);
        m_device = device;

        VkShaderModule shader = create_shader_module(m_device, spv);

        VkPipelineShaderStageCreateInfo stages[] = {
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = shader,
                .pName = config.vert_entry.data(),
            },
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = shader,
                .pName = config.frag_entry.data(),
            },
        };

        VkPushConstantRange push_range = {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(glm::mat4),
        };

        VkPipelineLayoutCreateInfo layout_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &push_range,
        };
        vk_verify(vkCreatePipelineLayout(device, &layout_info, nullptr,
                                         &m_pipeline_layout));


        VkVertexInputBindingDescription binding = {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        std::array<VkVertexInputAttributeDescription, 1> attrs = {{
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, position),
            },
        }};

        VkPipelineVertexInputStateCreateInfo vertex_input = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &binding,
            .vertexAttributeDescriptionCount = static_cast<u32>(attrs.size()),
            .pVertexAttributeDescriptions = attrs.data(),
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly = {
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        };
        VkPipelineRasterizationStateCreateInfo rasterization = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.0f,
        };
        VkPipelineMultisampleStateCreateInfo multisample = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };
        VkPipelineColorBlendAttachmentState blend_attachment = {
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT,
        };
        VkPipelineColorBlendStateCreateInfo color_blend = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &blend_attachment,
        };
        VkDynamicState dynamic_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo dynamic = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamic_states,
        };
        VkPipelineViewportStateCreateInfo viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };
        VkPipelineRenderingCreateInfo rendering = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &config.color_format,
            .depthAttachmentFormat = config.depth_format,
        };

        VkPipelineDepthStencilStateCreateInfo depth_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
        };

        VkGraphicsPipelineCreateInfo pipeline_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &rendering,
            .stageCount = 2,
            .pStages = stages,
            .pVertexInputState = &vertex_input,
            .pInputAssemblyState = &input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterization,
            .pMultisampleState = &multisample,
            .pDepthStencilState = &depth_info,
            .pColorBlendState = &color_blend,
            .pDynamicState = &dynamic,
            .layout = m_pipeline_layout,
        };

        vk_verify(vkCreateGraphicsPipelines(
            device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline));

        destroy_shader_module(device, shader);


        m_is_initialized = true;
        spdlog::info("Vulkan graphics pipeline created");
    }

    void VulkanGraphicsPipeline::destroy() {
        if (m_is_initialized) {
            if (m_pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_device, m_pipeline, nullptr);
                m_pipeline = VK_NULL_HANDLE;
            }
            if (m_pipeline_layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
                m_pipeline_layout = VK_NULL_HANDLE;
            }
            spdlog::info("Vulkan graphics pipeline destroyed");
            m_is_initialized = false;
        }
    }

    void VulkanGraphicsPipeline::bind(VkCommandBuffer cmd) const {
        check(m_is_initialized);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

    VkPipeline VulkanGraphicsPipeline::get_pipeline() const {
        check(m_is_initialized);
        return m_pipeline;
    }

    VkPipelineLayout VulkanGraphicsPipeline::get_layout() const {
        check(m_is_initialized);
        return m_pipeline_layout;
    }

} // namespace mantle
