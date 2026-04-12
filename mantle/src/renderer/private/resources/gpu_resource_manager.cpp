#include "renderer/gpu_resource_manager.h"

// TODO
namespace mantle {
    GPUResourceManager::~GPUResourceManager() {}

    ShaderHandle
    GPUResourceManager::create_shader(std::pmr::vector<u32> spir_v) {}

    void GPUResourceManager::destroy_shader(ShaderHandle shader) {}

    GraphicsPipelineHandle GPUResourceManager::create_graphics_pipeline(
        const GraphicsPipelineDesc &desc) {}

    ComputePipelineHandle GPUResourceManager::create_compute_pipeline(
        const ComputePipelineDesc &desc) {}

    void GPUResourceManager::destroy_graphics_pipeline(
        GraphicsPipelineHandle pipeline) {}

    void GPUResourceManager::destroy_compute_pipeline(
        ComputePipelineHandle pipeline) {}

    BufferHandle GPUResourceManager::create_buffer(const BufferDesc &desc) {}

    void GPUResourceManager::destroy_buffer(BufferHandle buffer) {}

    ImageHandle GPUResourceManager::create_image(const ImageDesc &desc) {}

    void GPUResourceManager::destroy_image(ImageHandle image) {}
} // namespace mantle
