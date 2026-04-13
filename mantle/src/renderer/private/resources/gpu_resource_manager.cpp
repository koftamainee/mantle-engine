#include "renderer/gpu_resource_manager.h"

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
} // namespace mantle
