#pragma once
#include <string_view>
#include <vector>

#include <renderer/types.h>
#include "core/memory/arena_allocator.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/memory/scope_arena.h"
#include "gpu_resource_manager.h"

namespace mantle {
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

        void begin_rendering(const RGRenderingInfo &info);
        void end_rendering();

        void draw(const RGDrawInfo &info);
        void draw_indexed(const RGDrawIndexedInfo &info);

        void dispatch(const RGDispatchInfo &info);

        void copy_buffer(const RGBufferCopyInfo &info);
        void copy_buffer_to_image(const RGBufferImageCopyInfo &info);

        /* TODO:
         * 1. add copy_image and blit_image, copy_image_to_buffer,set_viewport,
         * set_scissors, clear_color_image, clear_depth_image,
         * bind_vertex_buffer, bind_index_buffer
         *
         * 2. Change push_constants signature to accept info struct
         */

        void push_constants(const void *data, u32 size, u32 offset = 0);

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
