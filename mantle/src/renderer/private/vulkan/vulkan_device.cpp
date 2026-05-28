#include "../vulkan/vulkan_device.h"

#include <algorithm>
#include <core/assert.h>

#include <unordered_set>
#include "vulkan/vkassert.h"
#include "vulkan/vulkan_types.h"

#include <spdlog/spdlog.h>
#include "core/memory/scope_arena.h"


namespace mantle {

    VulkanDevice::~VulkanDevice() { destroy(); }

    void VulkanDevice::init(VkInstance instance, VkSurfaceKHR surface,
                            VkAllocationCallbacks *vk_callbacks,
                            VirtualHeap *heap, ArenaAllocator *scratch_arena) {
        check(!m_is_initialized);

        m_alloc_callbacks = vk_callbacks;
        m_resource = PersistentResource(heap);
        m_scratch_arena = scratch_arena;
        m_scratch_resource = ArenaResource(m_scratch_arena);


        m_queue_family_properties =
            std::pmr::vector<VkQueueFamilyProperties>(&m_resource);
        m_supported_extensions =
            std::pmr::vector<std::pmr::string>(&m_resource);

        create_physical_device(instance, surface);

        vkGetPhysicalDeviceProperties(m_physical_device, &m_properties);
        vkGetPhysicalDeviceFeatures(m_physical_device, &m_features);
        vkGetPhysicalDeviceMemoryProperties(m_physical_device,
                                            &m_memory_properties);
        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device,
                                                 &queue_family_count, nullptr);

        fatal(queue_family_count == 0, "Failed to get family count");

        m_queue_family_properties.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(
            m_physical_device, &queue_family_count,
            m_queue_family_properties.data());

        u32 extension_count = 0;
        vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr,
                                             &extension_count, nullptr);

        if (extension_count > 0) {
            ScopeArena scope(m_scratch_arena);
            std::pmr::vector<VkExtensionProperties> extensions(
                extension_count, &m_scratch_resource);

            if (vkEnumerateDeviceExtensionProperties(
                    m_physical_device, nullptr, &extension_count,
                    extensions.data()) == VK_SUCCESS) {
                m_supported_extensions.reserve(extension_count);

                for (const auto &ext : extensions) {
                    m_supported_extensions.emplace_back(ext.extensionName);
                }
            }
        }

        create_logical_device(instance);

        m_is_initialized = true;

        m_command_pool = create_command_pool(m_queue_indices.graphics_family);
        m_transfer_command_pool =
            create_command_pool(m_queue_indices.transfer_family);
        spdlog::info("Logical device command pool created");
    }

    void VulkanDevice::destroy() {
        if (m_is_initialized) {
            if (m_command_pool != VK_NULL_HANDLE) {
                check(m_device != VK_NULL_HANDLE);
                vkDestroyCommandPool(m_device, m_command_pool,
                                     m_alloc_callbacks);
                m_command_pool = VK_NULL_HANDLE;
            }
            if (m_transfer_command_pool != VK_NULL_HANDLE) {
                check(m_device != VK_NULL_HANDLE);
                vkDestroyCommandPool(m_device, m_transfer_command_pool,
                                     m_alloc_callbacks);
                m_transfer_command_pool = VK_NULL_HANDLE;
            }
            destroy_logical_device();
            destroy_physical_device();

            m_properties = {};
            m_features = {};
            m_enabled_features = {};
            m_memory_properties = {};
            m_queue_family_properties.resize(0);
            m_supported_extensions.resize(0);

            m_alloc_callbacks = nullptr;
            m_is_initialized = false;
        }
    }

    VkDevice VulkanDevice::get_device() const {
        check(m_is_initialized);
        return m_device;
    }

    VkPhysicalDevice VulkanDevice::get_physical_device() const {
        check(m_is_initialized);
        return m_physical_device;
    }

    SwapchainSupportDetails VulkanDevice::get_swapchain_support_details(
        VkSurfaceKHR surface, std::pmr::memory_resource *pmr) const {
        check(m_is_initialized);
        check(surface != VK_NULL_HANDLE);
        check(m_physical_device != VK_NULL_HANDLE);

        SwapchainSupportDetails details;

        details.formats = std::pmr::vector<VkSurfaceFormatKHR>(pmr);
        details.present_modes = std::pmr::vector<VkPresentModeKHR>(pmr);

        vk_verify(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            m_physical_device, surface, &details.capabilities));

        u32 format_count = 0;
        vk_verify(vkGetPhysicalDeviceSurfaceFormatsKHR(
            m_physical_device, surface, &format_count, nullptr));

        details.formats.resize(format_count);
        vk_verify(vkGetPhysicalDeviceSurfaceFormatsKHR(
            m_physical_device, surface, &format_count, details.formats.data()));

        u32 present_mode_count = 0;
        vk_verify(vkGetPhysicalDeviceSurfacePresentModesKHR(
            m_physical_device, surface, &present_mode_count, nullptr));

        details.present_modes.resize(present_mode_count);
        vk_verify(vkGetPhysicalDeviceSurfacePresentModesKHR(
            m_physical_device, surface, &present_mode_count,
            details.present_modes.data()));

        return details;
    }

    u32 VulkanDevice::get_queue_family_index(VkQueueFlags queue_flags) const {
        const auto queue_family_count =
            static_cast<u32>(m_queue_family_properties.size());

        if (queue_flags & VK_QUEUE_COMPUTE_BIT) {
            for (usize i = 0; i < queue_family_count; i++) {
                const auto &props = m_queue_family_properties[i];

                if ((props.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                    !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                    return i;
                }
            }
        }

        if (queue_flags & VK_QUEUE_TRANSFER_BIT) {
            for (usize i = 0; i < queue_family_count; i++) {
                const auto &props = m_queue_family_properties[i];

                if ((props.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                    !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                    !(props.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                    return i;
                }
            }
        }

        for (usize i = 0; i < queue_family_count; i++) {
            const auto &props = m_queue_family_properties[i];

            if ((props.queueFlags & queue_flags) == queue_flags) {
                return i;
            }
        }

        fatal(true, "Could not find a matching queue family index");
    }

    std::optional<u32>
    VulkanDevice::get_memory_type(u32 type_bits,
                                  VkMemoryPropertyFlags properties) const {
        check(m_is_initialized);
        for (usize i = 0; i < m_memory_properties.memoryTypeCount; i++) {
            if ((type_bits & 1u) == 1u) {
                const auto &memory_type = m_memory_properties.memoryTypes[i];

                if ((memory_type.propertyFlags & properties) == properties) {
                    return i;
                }
            }

            type_bits >>= 1u;
        }

        return std::nullopt;
    }

    VkResult VulkanDevice::copy_buffer(VkBuffer src, VkBuffer dst,
                                       VkQueue queue, VkDeviceSize size,
                                       VkDeviceSize src_offset,
                                       VkDeviceSize dst_offset) const {
        check(m_is_initialized);
        VkCommandBuffer cmd = create_command_buffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_transfer_command_pool, true);

        VkBufferCopy region = {
            .srcOffset = src_offset,
            .dstOffset = dst_offset,
            .size = size,
        };

        vkCmdCopyBuffer(cmd, src, dst, 1, &region);

        flush_command_buffer(cmd, queue, m_transfer_command_pool, true);

        return VK_SUCCESS;
    }


    VkCommandPool VulkanDevice::create_command_pool(
        u32 queue_family_index, VkCommandPoolCreateFlags create_flags) const {
        check(m_is_initialized);
        check(m_device != VK_NULL_HANDLE);

        VkCommandPoolCreateInfo cmd_pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = create_flags,
            .queueFamilyIndex = queue_family_index};
        VkCommandPool pool;
        vk_verify(vkCreateCommandPool(m_device, &cmd_pool_create_info,
                                      m_alloc_callbacks, &pool));
        return pool;
    }

    VkCommandBuffer
    VulkanDevice::create_command_buffer(VkCommandBufferLevel level,
                                        VkCommandPool pool, bool begin) const {
        check(m_is_initialized);
        check(m_device != VK_NULL_HANDLE);
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .level = level,
            .commandBufferCount = 1,
        };
        VkCommandBuffer cmd_buffer;
        vk_verify(vkAllocateCommandBuffers(
            m_device, &command_buffer_allocate_info, &cmd_buffer));

        if (begin) {
            VkCommandBufferBeginInfo begin_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            };
            vk_verify(vkBeginCommandBuffer(cmd_buffer, &begin_info));
        }

        return cmd_buffer;
    }

    VkCommandBuffer
    VulkanDevice::create_command_buffer(VkCommandBufferLevel level,
                                        bool begin) const {
        check(m_is_initialized);
        return create_command_buffer(level, m_command_pool, begin);
    }

    void VulkanDevice::flush_command_buffer(VkCommandBuffer command_buffer,
                                            VkQueue queue, VkCommandPool pool,
                                            bool free) const {
        check(m_is_initialized);
        check(m_device != VK_NULL_HANDLE);

        if (command_buffer == VK_NULL_HANDLE) {
            return;
        }

        vk_verify(vkEndCommandBuffer(command_buffer));
        VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                    .commandBufferCount = 1,
                                    .pCommandBuffers = &command_buffer};

        VkFenceCreateInfo fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        VkFence fence;

        vk_verify(vkCreateFence(m_device, &fence_create_info, m_alloc_callbacks,
                                &fence));
        vk_verify(vkQueueSubmit(queue, 1, &submit_info, fence));

        vk_verify(vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(m_device, fence, m_alloc_callbacks);

        if (free) {
            vkFreeCommandBuffers(m_device, pool, 1, &command_buffer);
        }
    }

    void VulkanDevice::flush_command_buffer(VkCommandBuffer command_buffer,
                                            VkQueue queue, bool free) const {
        check(m_is_initialized);

        flush_command_buffer(command_buffer, queue, m_command_pool, free);
    }

    VkCommandBuffer
    VulkanDevice::begin_single_time_commands(VkCommandPool pool) const {
        VkCommandBuffer cmd =
            create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, pool, false);

        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

        vk_verify(vkBeginCommandBuffer(cmd, &begin_info));

        return cmd;
    }

    void VulkanDevice::end_single_time_commands(VkCommandBuffer command_buffer,
                                                VkQueue queue,
                                                VkCommandPool pool) const {
        flush_command_buffer(command_buffer, queue, pool, true);
    }

    VkCommandBuffer VulkanDevice::begin_single_time_commands() const {
        return begin_single_time_commands(m_command_pool);
    }

    void VulkanDevice::end_single_time_commands(VkCommandBuffer command_buffer,
                                                VkQueue queue) const {
        flush_command_buffer(command_buffer, queue, m_command_pool, true);
    }


    bool VulkanDevice::extension_supported(std::string_view extension) const {
        check(m_is_initialized);
        return std::ranges::contains(m_supported_extensions, extension);
    }

    VkFormat VulkanDevice::get_supported_depth_format(
        bool check_sampling_support) const {
        check(m_is_initialized);
        constexpr std::array<VkFormat, 5> depth_formats = {
            VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM};

        for (const auto &format : depth_formats) {
            VkFormatProperties format_properties{};
            vkGetPhysicalDeviceFormatProperties(m_physical_device, format,
                                                &format_properties);

            if (format_properties.optimalTilingFeatures &
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                if (check_sampling_support) {
                    if (!(format_properties.optimalTilingFeatures &
                          VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
                        continue;
                    }
                }

                return format;
            }
        }

        fatal(true, "Could not find a matching depth format");
    }

    VkQueue VulkanDevice::get_graphics_queue() const {
        check(m_is_initialized);
        return m_graphics_queue;
    }

    VkQueue VulkanDevice::get_present_queue() const {
        check(m_is_initialized);
        return m_present_queue;
    }

    VkQueue VulkanDevice::get_transfer_queue() const {
        check(m_is_initialized);
        return m_transfer_queue;
    }

    QueueFamilyIndices VulkanDevice::get_queue_families() const {
        check(m_is_initialized);
        return m_queue_indices;
    }

    void VulkanDevice::create_physical_device(VkInstance instance,
                                              VkSurfaceKHR surface) {
        check(instance != VK_NULL_HANDLE);

        u32 device_count = 0;
        vk_verify(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
        fatal(device_count == 0, "Failed to enumerate physical devices");

        ScopeArena scope(m_scratch_arena);
        std::pmr::vector<VkPhysicalDevice> devices(device_count,
                                                   &m_scratch_resource);
        vk_verify(vkEnumeratePhysicalDevices(instance, &device_count,
                                             devices.data()));

        for (const auto &physical_device_candidate : devices) {
            if (is_physical_device_suitable(physical_device_candidate, surface,
                                            m_queue_indices)) {
                m_physical_device = physical_device_candidate;
                spdlog::info("Physical Device created");
                break;
            }
        }

        fatal(m_physical_device == VK_NULL_HANDLE,
              "Supported physical Device not found");
    }

    void VulkanDevice::destroy_physical_device() {
        if (m_physical_device != VK_NULL_HANDLE) {
            m_physical_device = VK_NULL_HANDLE;
            spdlog::info("Physical device destroyed");
        }
    }

    void VulkanDevice::create_logical_device(VkInstance instance) {
        check(instance != VK_NULL_HANDLE);
        check(m_physical_device != VK_NULL_HANDLE);

        std::unordered_set unique_queue_families = {
            m_queue_indices.graphics_family,
            m_queue_indices.present_family,
            m_queue_indices.transfer_family,
        };

        f32 queue_priority = 0.5f;

        ScopeArena scope(m_scratch_arena);
        std::pmr::vector<VkDeviceQueueCreateInfo> queue_create_infos(
            &m_scratch_resource);
        for (auto queue_family : unique_queue_families) {
            VkDeviceQueueCreateInfo queue_create_info = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_family,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            };
            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures2 features2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .features = {.samplerAnisotropy = VK_TRUE, .shaderInt16 = VK_TRUE,},
        };
        VkPhysicalDeviceVulkan11Features vulkan11_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .uniformAndStorageBuffer16BitAccess = VK_TRUE,
            .shaderDrawParameters = VK_TRUE,
        };

        VkPhysicalDeviceDescriptorIndexingFeatures indexing = {};
        indexing.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        indexing.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        indexing.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        indexing.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        indexing.descriptorBindingPartiallyBound = VK_TRUE;
        indexing.runtimeDescriptorArray = VK_TRUE;

        VkPhysicalDeviceVulkan13Features vulkan13_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = nullptr,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        };


        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamic_state_features = {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = nullptr,
        };

        features2.pNext = &vulkan11_features;
        vulkan11_features.pNext = &indexing;
        indexing.pNext = &vulkan13_features;
        vulkan13_features.pNext = &dynamic_state_features;
        dynamic_state_features.pNext = nullptr;

        VkDeviceCreateInfo device_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features2,
            .queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledExtensionCount =
                static_cast<u32>(ms_device_extensions.size()),
            .ppEnabledExtensionNames = ms_device_extensions.data(),
        };

        vk_verify(vkCreateDevice(m_physical_device, &device_create_info,
                                 m_alloc_callbacks, &m_device));


        vkGetDeviceQueue(m_device, m_queue_indices.graphics_family, 0,
                         &m_graphics_queue);
        vkGetDeviceQueue(m_device, m_queue_indices.present_family, 0,
                         &m_present_queue);
        vkGetDeviceQueue(m_device, m_queue_indices.transfer_family, 0,
                         &m_transfer_queue);

        spdlog::info("Logical device created");
    }

    void VulkanDevice::destroy_logical_device() {
        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, m_alloc_callbacks);

            m_device = VK_NULL_HANDLE;

            m_queue_indices = QueueFamilyIndices{};

            spdlog::info("Logical device destroyed");
        }
    }

    bool VulkanDevice::is_physical_device_suitable(
        VkPhysicalDevice physical_device, VkSurfaceKHR surface,
        QueueFamilyIndices &queue_family_indices) {
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_device, &features);

        if (!features.geometryShader)
            return false;
        if (!features.samplerAnisotropy)
            return false;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        if (properties.apiVersion < VK_API_VERSION_1_3)
            return false;

        if (!is_physical_device_supports_required_extensions(physical_device)) {
            return false;
        }

        u32 format_count = 0;
        vk_verify(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                                       &format_count, nullptr));
        if (format_count == 0)
            return false;

        u32 present_mode_count = 0;
        vk_verify(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, surface, &present_mode_count, nullptr));
        if (present_mode_count == 0)
            return false;

        const QueueFamilyIndices indices =
            find_queue_families(physical_device, surface);

        if (!indices.is_complete())
            return false;

        queue_family_indices = indices;

        return true;
    }

    bool VulkanDevice::is_physical_device_supports_required_extensions(
        VkPhysicalDevice physical_device) {
        u32 extensions_count = 0;
        vk_verify(vkEnumerateDeviceExtensionProperties(
            physical_device, nullptr, &extensions_count, nullptr));

        ScopeArena scope(m_scratch_arena);
        std::pmr::vector<VkExtensionProperties> extensions(extensions_count,
                                                           &m_scratch_resource);
        vk_verify(vkEnumerateDeviceExtensionProperties(
            physical_device, nullptr, &extensions_count, extensions.data()));

        for (const char *required : ms_device_extensions) {
            bool found = false;

            for (usize i = 0; i < extensions_count; i++) {
                if (std::strcmp(required, extensions[i].extensionName) == 0) {
                    found = true;
                    spdlog::trace("Found required device extension: {}",
                                  required);
                    break;
                }
            }

            if (!found)
                return false;
        }

        return true;
    }

    QueueFamilyIndices
    VulkanDevice::find_queue_families(VkPhysicalDevice physical_device,
                                      VkSurfaceKHR surface) {
        check(physical_device != VK_NULL_HANDLE);
        check(surface != VK_NULL_HANDLE);

        QueueFamilyIndices indices;

        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                                 &queue_family_count, nullptr);

        ScopeArena scope(m_scratch_arena);
        std::pmr::vector<VkQueueFamilyProperties> queue_families(
            queue_family_count, &m_scratch_resource);
        vkGetPhysicalDeviceQueueFamilyProperties(
            physical_device, &queue_family_count, queue_families.data());

        for (usize i = 0; i < queue_family_count; i++) {
            if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                indices.graphics_family = i;
                break;
            }
        }

        for (usize i = 0; i < queue_family_count; i++) {
            const auto &props = queue_families[i];

            if ((props.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                indices.transfer_family = i;
                break;
            }
        }

        for (usize i = 0; i < queue_family_count; i++) {
            VkBool32 present_support = VK_FALSE;

            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                                 &present_support);

            if (present_support) {
                indices.present_family = i;
                break;
            }
        }

        if (indices.transfer_family == UINT32_MAX) {
            spdlog::trace("No separate transfer queue found. Fallback transfer "
                          "queue = graphics_queue");
            indices.transfer_family = indices.graphics_family;
        }

        return indices;
    }

} // namespace mantle
