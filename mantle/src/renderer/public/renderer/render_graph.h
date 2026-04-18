#pragma once
#include <string_view>
#include <vector>

#include <renderer/types.h>
#include <sys/stat.h>

#include "core/memory/arena_allocator.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/memory/scope_arena.h"
#include "gpu_resource_manager.h"

namespace mantle {
    class RenderGraph;

    // TODO: Setup phase

    // TODO: TransientResources system:
    // TODO: It will do heavy-lifting of allocating resources and aliasing it
    // TODO: for render graph

    // TODO: Deferred created resources:
    // TODO: Create new resource in render graph only when it used
    // TODO: for the first time, not when it was declared.

    // TODO: Derived resource parameters:
    // TODO: Create render pass output based on input size / format
    // TODO: Derive bind flags from a usage
    // TODO: Example: Downsampling render pass which would produce
    // TODO: half-resolution version of input resource

    // TODO: MoveSubresource:
    // TODO: Reuse already existing resource for future render passes instead of
    // TODO: allocating new one

    // TODO: Compilation phase
    // TODO: Hidden from used, auto runs when graph is passed for execution

    // TODO: 1. Cull resources and passes that are declared, but now used
    // TODO: 2. Calculate resources lifetime
    // TODO: 3. Greedy allocate resources needed for current frame

    // TODO: Execution phase:
    // TODO: Renderer::execute(RenderGraph &render_graph);

    // TODO: 1. Execute callback functions for each pass
    // TODO: 2. Use immediate rendering code style using RenderPassContext
    // TODO: 3. Get real GPU resources from handles, generated in setup phase

    // TODO: Async compute:
    // TODO: check more on it, probably introduce opt-in render passes that can
    // TODO: be executed async from main queue

    // TODO: Blackboard:
    // TODO: Blackboard::add<T>();
    // TODO: Blackboard::get<T>()
    // TODO: Allow communication between modules via hash table of components

    class RenderGraphBuilder final {
      public:
        RenderGraphBuilder() = delete;
        ~RenderGraphBuilder();

        RenderGraphBuilder(const RenderGraphBuilder &other) = delete;
        RenderGraphBuilder(RenderGraphBuilder &&other) noexcept = delete;
        RenderGraphBuilder &operator=(const RenderGraphBuilder &other) = delete;
        RenderGraphBuilder &
        operator=(RenderGraphBuilder &&other) noexcept = delete;

        // TODO: add async_compute_enable(true);
        // TODO: add use_render_target(<something>);

        // TODO: add initial state for images (clear / undefined) and probably
        // TODO: something for buffers too
        RGImageHandle create_image(const ImageDesc &desc);
        RGBufferHandle create_buffer(const BufferDesc &desc);

        // TODO: add read_flags and write_flags
        RGImageHandle read(RGImageHandle image);
        RGBufferHandle read(RGBufferHandle buffer);

        // TODO: writes should invalidate old handle and create new ones
        // TODO: referencing invalidated resources will produce runtime error
        RGImageHandle write(RGImageHandle image);
        RGBufferHandle write(RGBufferHandle buffer);

      private:
        // TODO
    };

    class RenderPassContext final {
      public:
        RenderPassContext() = delete;
        ~RenderPassContext();

        RenderPassContext(const RenderPassContext &other) = delete;
        RenderPassContext(RenderPassContext &&other) noexcept = delete;
        RenderPassContext &operator=(const RenderPassContext &other) = delete;
        RenderPassContext &
        operator=(RenderPassContext &&other) noexcept = delete;

        void bind_pipeline(GraphicsPipelineHandle pipeline);
        void bind_pipeline(ComputePipelineHandle pipeline);

        void begin_rendering(const RGRenderingInfo &info);
        void end_rendering();

        void set_viewport(f32 x, f32 y, f32 width, f32 height);
        void set_scissor(i32 x, i32 y, u32 width, u32 height);

        void bind_vertex_buffer(RGBufferHandle buffer, u32 binding,
                                usize offset = 0);
        void bind_index_buffer(RGBufferHandle buffer, usize offset = 0);

        void draw(const RGDrawInfo &info);
        void draw_indexed(const RGDrawIndexedInfo &info);
        void dispatch(const RGDispatchInfo &info);

        void copy_buffer(const RGBufferCopyInfo &info);
        void copy_buffer_to_image(const RGBufferImageCopyInfo &info);
        void copy_image_to_buffer(const RGImageBufferCopyInfo &info);
        void copy_image(const RGImageCopyInfo &info);
        void blit_image(const RGImageBlitInfo &info);

        void clear_color_image(RGImageHandle image, f32 r, f32 g, f32 b, f32 a);
        void clear_depth_image(RGImageHandle image, f32 depth);

        void push_constants(const void *data, ShaderStage stage);

      private:
        struct Impl;

        friend class RenderGraph;
        void init();
        void destroy();

        Impl *m_impl;
    };

    template <typename TData>
    concept CRenderPassData = std::is_default_constructible_v<TData>;

    template <typename TData, typename TSetup>
    concept CRenderPassSetupLambda =
        requires(TSetup fn, RenderGraphBuilder &builder, TData &data) {
            { fn(builder, data) } -> std::same_as<void>;
        };

    template <typename TData, typename TExecute>
    concept CRenderPassExecuteLambda =
        requires(TExecute fn, const TData &data, RenderPassContext &ctx) {
            { fn(ctx, data) } -> std::same_as<void>;
        };

    class RenderGraph final {
      public:
        explicit RenderGraph(ArenaAllocator *arena);

        template <typename TData, typename TSetup, typename TExecute>
            requires CRenderPassData<TData> &&
            CRenderPassSetupLambda<TData, TSetup> &&
            CRenderPassExecuteLambda<TData, TExecute>
        const TData &add_pass(std::string_view name, TSetup &&setup,
                              TExecute &&execute) {
            return {};
        }

        RGImageHandle import_image(ImageHandle image);
        RGBufferHandle import_buffer(BufferHandle buffer);


      private:
        struct RenderPassNode {
            std::string_view name;
            void *execute_data = nullptr;
            void (*execute_fn)(void *data, RenderPassContext &ctx) = nullptr;
        };


        ArenaAllocator *m_arena = nullptr;
        ScopeArena m_scope;
        ArenaResource m_resource{};
        std::pmr::vector<RenderPassNode> m_passes;
    };
} // namespace mantle
