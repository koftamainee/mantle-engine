#pragma once
#include <memory>
#include <span>
#include "mesh/mesh_handle.h"
#include "mesh/vertex.h"

namespace mantle {
    class Renderer;
    class VulkanResourceManager;
    class VulkanDevice;

    class GPUResourceManager final {
    public:
        GPUResourceManager(const GPUResourceManager&) = delete;
        GPUResourceManager& operator=(const GPUResourceManager&) = delete;
        GPUResourceManager(GPUResourceManager&&) noexcept = delete;
        GPUResourceManager& operator=(GPUResourceManager&&) noexcept = delete;

        ~GPUResourceManager() = default;

        MeshHandle upload_mesh(std::span<const Vertex> vertices,
                               std::span<const uint32_t> indices) const;
        void destroy_mesh(MeshHandle handle) const;

        bool is_valid(MeshHandle handle) const;

    private:
        friend class Renderer;

        bool m_is_initialized = false;

        GPUResourceManager() = default;

        struct Impl;
        void init(VulkanResourceManager& vulkan_resources, VulkanDevice& device);

        std::unique_ptr<Impl> m_impl;
    };
}