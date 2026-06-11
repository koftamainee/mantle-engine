// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <vulkan/command_recorder.h>

#include "mantle/core/memory/arena_allocator.h"
#include "mantle/core/memory/pmr/arena_resource.h"
#include "mantle/renderer/frame_graph.h"
#include "mantle/renderer/types.h"
#include "resources/gpu_resource_manager_internal.h"
#include "resources/transient_resources.h"

namespace mantle {
    struct FGPassContext::Impl {
        CommandRecorder *cmd;

        GPUResourceManager *resource_manager = nullptr;

        TransientResources *transient_resources = nullptr;

        ArenaAllocator *scratch_arena = nullptr;
        ArenaResource  *scratch_resource = nullptr;
    };
} // namespace mantle
