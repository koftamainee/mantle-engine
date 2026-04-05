#pragma once
#include <span>
#include <string_view>
#include <vulkan/vulkan_core.h>

namespace mantle {
    class VulkanGraphicsPipeline final {
    public:
        struct Config final {
            std::string_view vert_entry = "vert_main";
            std::string_view frag_entry = "frag_main";
            VkFormat color_format{};
        };

        VulkanGraphicsPipeline() = default;
        ~VulkanGraphicsPipeline();

        VulkanGraphicsPipeline(const VulkanGraphicsPipeline &) = delete;
        VulkanGraphicsPipeline &operator=(const VulkanGraphicsPipeline &) = delete;
        VulkanGraphicsPipeline(VulkanGraphicsPipeline &&) noexcept = delete;
        VulkanGraphicsPipeline &operator=(VulkanGraphicsPipeline &&) noexcept = delete;

        void init(VkDevice device, const Config &config, std::span<uint32_t> spv);
        void destroy();

        void bind(VkCommandBuffer cmd) const;

        VkPipeline get_pipeline() const;
        VkPipelineLayout get_layout() const;

    private:
        bool m_is_initialized = false;

        VkDevice m_device{};
        VkPipeline m_pipeline{};
        VkPipelineLayout m_pipeline_layout{};
    };
}
