#include "../vulkan/vkassert.h"
#include "../resources/vulkan_resource_manager.h"
#include "core/assert.h"

namespace mantle {
    VulkanResourceManager::~VulkanResourceManager() { destroy(); }

    void VulkanResourceManager::init(VkPhysicalDevice physical_device, VkDevice device, VkInstance instance) {
        check(!m_is_initialized);
        m_allocator.init(physical_device, device, instance);
        m_buffers.clear();
        m_images.clear();

        m_buffer_free_list.clear();
        m_image_free_list.clear();

        m_buffer_generations.clear();
        m_image_generations.clear();

        m_is_initialized = true;
        spdlog::info("Vulkan resource manager is initialized");
    }

    void VulkanResourceManager::destroy() {
        if (m_is_initialized) {
            for (auto buffer : m_buffers) {
                m_allocator.destroy_buffer(buffer.buffer, buffer.allocation);
            }
            for (auto image : m_images) {
                m_allocator.destroy_image(image.image, image.allocation);
            }
            m_buffers.clear();
            m_images.clear();

            m_buffer_free_list.clear();
            m_image_free_list.clear();

            m_buffer_generations.clear();
            m_image_generations.clear();

            m_allocator.destroy();
            m_is_initialized = false;
            spdlog::info("Vulkan resource manager is destroyed");
        }
    }

    VulkanResourceManager::ResourceHandle VulkanResourceManager::create_buffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, void **mapped_data) {
        check(m_is_initialized);

        VkBuffer buffer;
        VmaAllocation allocation;
        vk_verify(m_allocator.create_buffer(size, usage, memory_usage, &buffer, &allocation, mapped_data));

        ResourceID id;

        if (!m_buffer_free_list.empty()) {
            id = m_buffer_free_list.back();
            m_buffer_free_list.pop_back();

            m_buffers[id] = {buffer, allocation};
            m_buffer_generations[id]++;
        }
        else {
            id = static_cast<ResourceID>(m_buffers.size());

            m_buffers.push_back({buffer, allocation});
            m_buffer_generations.push_back(1);
        }

        return ResourceHandle{
            id,
            ResourceType::Buffer,
            m_buffer_generations[id]
        };

    }

    template <typename TData>
    TData &VulkanResourceManager::get_resource_data(
        ResourceHandle handle,
        ResourceType expected_type,
        std::vector<TData> &storage,
        const std::vector<uint32_t> &generations
        ) {
        check(m_is_initialized);
        check(handle.type == expected_type);
        check(handle.id < storage.size());
        check(handle.generation == generations[handle.id]);
        return storage[handle.id];
    }

    template <typename TData>
    const TData &VulkanResourceManager::get_resource_data(
        ResourceHandle handle,
        ResourceType expected_type,
        const std::vector<TData> &storage,
        const std::vector<uint32_t> &generations
        ) const {
        check(m_is_initialized);
        check(handle.type == expected_type);
        check(handle.id < storage.size());
        check(handle.generation == generations[handle.id]);
        return storage[handle.id];
    }

    VkBuffer VulkanResourceManager::get_buffer(ResourceHandle handle) const {
        return get_resource_data(handle, ResourceType::Buffer, m_buffers, m_buffer_generations).buffer;
    }

    void VulkanResourceManager::destroy_buffer(ResourceHandle handle, bool immediate) {
        auto &buf = get_resource_data(handle, ResourceType::Buffer, m_buffers, m_buffer_generations);

        if (buf.buffer == VK_NULL_HANDLE) {
            return;
        }

        if (immediate) {
            m_allocator.destroy_buffer(buf.buffer, buf.allocation);
        }
        else {
            m_deletion_queues[m_current_frame].push(
                [this, buffer = buf.buffer, allocation = buf.allocation]() {
                    m_allocator.destroy_buffer(buffer, allocation);
                }
                );
        }

        buf.buffer = VK_NULL_HANDLE;
        buf.allocation = nullptr;
        m_buffer_free_list.push_back(handle.id);
        m_buffer_generations[handle.id]++;
    }

    VulkanResourceManager::ResourceHandle VulkanResourceManager::create_image(
        const VkImageCreateInfo &info,
        VmaMemoryUsage memory_usage
        ) {
        check(m_is_initialized);

        VkImage image;
        VmaAllocation allocation;

        vk_verify(m_allocator.create_image(info, memory_usage, &image, &allocation));

        ResourceID id;

        if (!m_image_free_list.empty()) {
            id = m_image_free_list.back();
            m_image_free_list.pop_back();

            m_images[id] = {image, allocation};
            m_image_generations[id]++;
        }
        else {
            id = static_cast<ResourceID>(m_images.size());

            m_images.push_back({image, allocation});
            m_image_generations.push_back(1);
        }

        return ResourceHandle{
            id,
            ResourceType::Image,
            m_image_generations[id]
        };
    }


    VkImage VulkanResourceManager::get_image(ResourceHandle handle) const {
        return get_resource_data(handle, ResourceType::Image, m_images, m_image_generations).image;
    }

    void VulkanResourceManager::destroy_image(ResourceHandle handle, bool immediate) {
        auto &img = get_resource_data(handle, ResourceType::Image, m_images, m_image_generations);

        if (img.image == VK_NULL_HANDLE) {
            return;
        }

        if (immediate) {
            m_allocator.destroy_image(img.image, img.allocation);
        }
        else {
            m_deletion_queues[m_current_frame].push(
                [this, image = img.image, allocation = img.allocation]() {
                    m_allocator.destroy_image(image, allocation);
                }
                );
        }

        img.image = VK_NULL_HANDLE;
        img.allocation = nullptr;
        m_image_free_list.push_back(handle.id);
        m_image_generations[handle.id]++;
    }

    void VulkanResourceManager::next_frame() {
        check(m_is_initialized);

        m_current_frame = (m_current_frame + 1) % ms_frame_lag;
        m_deletion_queues[m_current_frame].flush();
    }


}
