#pragma once
#include <vector>
#include "../resources/vulkan_resource_manager.h"
#include "../vulkan/vulkan_device.h"
#include "core/assert.h"
#include "renderer/gpu_resource_manager.h"

namespace mantle {
    struct GPUResourceManager::Impl final {
        struct MeshData {
            VulkanResourceManager::ResourceHandle vertex_buffer;
            VulkanResourceManager::ResourceHandle index_buffer;
            uint32_t index_count = 0;
        };
        MeshData& get_mesh_data(MeshHandle handle) {
            check(handle.id < meshes.size());
            check(handle.generation == generations[handle.id]);
            return meshes[handle.id];
        }

        VulkanResourceManager& vulkan_resources;
        VulkanDevice& device;

        std::vector<MeshData> meshes;
        std::vector<uint32_t> free_list;
        std::vector<uint32_t> generations;

        Impl(VulkanResourceManager& vulkan_resources, VulkanDevice& device)
            : vulkan_resources(vulkan_resources)
            , device(device) {}
    };
}
