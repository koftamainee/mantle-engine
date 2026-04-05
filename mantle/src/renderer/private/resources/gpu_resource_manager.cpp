#include "renderer/gpu_resource_manager.h"
#include "gpu_resource_manager_impl.h"
#include "core/assert.h"

namespace mantle {
    void GPUResourceManager::init(VulkanResourceManager &vulkan_resources, VulkanDevice &device) {
        check(!m_is_initialized);
        m_impl = std::make_unique<Impl>(vulkan_resources, device);
        m_is_initialized = true;
        spdlog::info("GPU resource manager is initialized");
    }

    MeshHandle GPUResourceManager::upload_mesh(
        std::span<const Vertex> vertices,
        std::span<const uint32_t> indices
        ) const {
        check(m_impl);
        check(m_is_initialized);

        const VkDeviceSize vertex_size = vertices.size_bytes();
        const VkDeviceSize index_size = indices.size_bytes();

        void *vertex_mapped = nullptr;
        void *index_mapped = nullptr;

        auto staging_vb = m_impl->vulkan_resources.create_buffer(
            vertex_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY,
            &vertex_mapped
            );
        auto staging_ib = m_impl->vulkan_resources.create_buffer(
            index_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY,
            &index_mapped
            );

        std::memcpy(vertex_mapped, vertices.data(), vertex_size);
        std::memcpy(index_mapped, indices.data(), index_size);

        auto vb = m_impl->vulkan_resources.create_buffer(
            vertex_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY
            );
        auto ib = m_impl->vulkan_resources.create_buffer(
            index_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY
            );

        VkQueue transfer_queue = m_impl->device.get_transfer_queue();

        m_impl->device.copy_buffer(
            m_impl->vulkan_resources.get_buffer(staging_vb),
            m_impl->vulkan_resources.get_buffer(vb),
            transfer_queue,
            vertex_size
            );
        m_impl->device.copy_buffer(
            m_impl->vulkan_resources.get_buffer(staging_ib),
            m_impl->vulkan_resources.get_buffer(ib),
            transfer_queue,
            index_size
            );

        m_impl->vulkan_resources.destroy_buffer(staging_vb, true);
        m_impl->vulkan_resources.destroy_buffer(staging_ib, true);

        Impl::MeshData mesh_data = {
            .vertex_buffer = vb,
            .index_buffer = ib,
            .index_count = static_cast<uint32_t>(indices.size()),
        };

        uint32_t id;
        if (!m_impl->free_list.empty()) {
            id = m_impl->free_list.back();
            m_impl->free_list.pop_back();
            m_impl->meshes[id] = mesh_data;
            m_impl->generations[id]++;
        }
        else {
            id = static_cast<uint32_t>(m_impl->meshes.size());
            m_impl->meshes.push_back(mesh_data);
            m_impl->generations.push_back(1);
        }

        return MeshHandle{id, m_impl->generations[id]};
    }

    void GPUResourceManager::destroy_mesh(MeshHandle handle) const {
        check(m_is_initialized);
        check(m_impl);
        check(handle.id < m_impl->meshes.size());
        check(handle.generation == m_impl->generations[handle.id]);

        auto &mesh = m_impl->meshes[handle.id];
        m_impl->vulkan_resources.destroy_buffer(mesh.vertex_buffer);
        m_impl->vulkan_resources.destroy_buffer(mesh.index_buffer);

        mesh = {};
        m_impl->free_list.push_back(handle.id);
        m_impl->generations[handle.id]++;
    }

    bool GPUResourceManager::is_valid(MeshHandle handle) const {
        check(m_is_initialized);
        check(m_impl);
        if (handle.id >= m_impl->meshes.size()) { return false; }
        return handle.generation == m_impl->generations[handle.id];
    }
}
