#include "renderer/gpu_resource_manager.h"

#include "gpu_resource_manager_internal.h"
#include "vulkan/vulkan_backend.h"

// TODO
namespace mantle {
    GPUResourceManager::~GPUResourceManager() {}

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

    BufferHandle GPUResourceManager::create_buffer(const BufferDesc &desc) {
        return {};
    }

    void GPUResourceManager::update_buffer(BufferHandle handle,
                                           const void *data, usize size,
                                           usize offset) {}

    void GPUResourceManager::destroy_buffer(BufferHandle buffer) {}

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

    void GPUResourceManager::init(VulkanBackend *backend) {}

    void GPUResourceManager::destroy() {}

    u32 GPUResourceManager::get_bindless_index(ImageHandle image) {}

    u32 GPUResourceManager::get_bindless_index(BufferHandle buffer) {}

    VkImage GPUResourceManager::Impl::get_vk_image(ImageHandle handle) const {}

    VkImageView
    GPUResourceManager::Impl::get_vk_image_view(ImageHandle handle) const {}

    VkBuffer
    GPUResourceManager::Impl::get_vk_buffer(BufferHandle handle) const {}

    VkPipeline GPUResourceManager::Impl::get_vk_pipeline(
        GraphicsPipelineHandle handle) const {}

    VkPipeline GPUResourceManager::Impl::get_vk_pipeline(
        ComputePipelineHandle handle) const {}

    VkPipelineLayout GPUResourceManager::Impl::get_vk_pipeline_layout(
        GraphicsPipelineHandle handle) const {}

    VkPipelineLayout GPUResourceManager::Impl::get_vk_pipeline_layout(
        ComputePipelineHandle handle) const {}

} // namespace mantle
