#pragma once
#include <string_view>
#include <vector>


#include "core/memory/arena_allocator.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/memory/scope_arena.h"
#include "gpu_resource_manager.h"

namespace mantle {
    struct RGBufferHandle final {
        u32 index;
    };
    struct RGImageHandle final {
        u32 index;
    };

    class RenderGraphBuilder final {
      public:
        RenderGraphBuilder() = delete;
        ~RenderGraphBuilder();

        RenderGraphBuilder(const RenderGraphBuilder &other) = delete;
        RenderGraphBuilder(RenderGraphBuilder &&other) noexcept = delete;
        RenderGraphBuilder &operator=(const RenderGraphBuilder &other) = delete;
        RenderGraphBuilder &
        operator=(RenderGraphBuilder &&other) noexcept = delete;

        RGImageHandle create_image(const ImageDesc &desc);
        RGBufferHandle create_buffer(const BufferDesc &desc);

        RGImageHandle read(RGImageHandle image);
        RGImageHandle write(RGImageHandle image);
        RGBufferHandle read(RGBufferHandle buffer);
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

        void draw(u32 vertex_count, u32 instance_count, u32 first_vertex,
                  u32 first_instance);
        void draw_indexed(u32 index_count, u32 instance_count, u32 first_index,
                          i32 vertex_offset, u32 first_instance);

        void draw_indirect(RGBufferHandle indirect_buffer, u32 offset,
                           u32 draw_count);
        void draw_indexed_indirect(RGBufferHandle indirect_buffer, u32 offset,
                                   u32 draw_count);

        void dispatch(u32 x, u32 y, u32 z);
        void push_constants(const void *data, u32 size);

      private:
        // TODO
    };
    class CompiledRenderGraph final {}; // TODO

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
            // TODO
            return {};
        }

        RGImageHandle import_image(ImageHandle image);
        RGBufferHandle import_buffer(BufferHandle buffer);

        CompiledRenderGraph compile(GPUResourceManager &resources);

      private:
        struct RenderPassNode {
            std::string_view name;
            void *execute_data = nullptr;
            void *execute_fn_ptr = nullptr;
            void (*execute_fn)(void *data, RenderPassContext &ctx) = nullptr;
        };


        ArenaAllocator *m_arena = nullptr;
        ScopeArena m_scope;
        ArenaResource m_resource{};
        std::pmr::vector<RenderPassNode> m_passes;
    };
} // namespace mantle
