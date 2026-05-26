#pragma once

#include "core/memory/arena_allocator.h"
#include "core/memory/pmr/arena_resource.h"
#include "renderer/render_graph.h"
#include "renderer/types.h"
#include "resources/gpu_resource_manager_internal.h"
#include "resources/transient_resources.h"
#include "vulkan/command_recorder.h"

namespace mantle {
    struct RenderPassContext::Impl {
        CommandRecorder *cmd;

        GPUResourceManager *resource_manager = nullptr;

        TransientResources *transient_resources = nullptr;

        ArenaAllocator *scratch_arena = nullptr;
        ArenaResource *scratch_resource = nullptr;
    };
} // namespace mantle
