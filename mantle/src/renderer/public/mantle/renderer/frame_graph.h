// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "mantle/renderer/types.h"

#include <string_view>
#include <vector>

#include "mantle/core/memory/arena_allocator.h"
#include "mantle/core/memory/pmr/arena_resource.h"
#include "mantle/core/memory/scope_arena.h"
#include "gpu_resource_manager.h"
#include "mantle/renderer/blackboard.h"

namespace mantle {
    class TransientResources;
    class Renderer;
} // namespace mantle

namespace mantle {
    class FrameGraph;

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

    // TODO: Async compute:
    // TODO: check more on it, probably introduce opt-in render passes that can
    // TODO: be executed async from main queue

    // TODO: Blackboard:
    // TODO: Blackboard::add<T>();
    // TODO: Blackboard::get<T>()
    // TODO: Allow communication between modules via hash table of components

    class FrameGraphBuilder final {
      public:
        FrameGraphBuilder() = default;
        ~FrameGraphBuilder() = default;

        MANTLE_NO_COPY_NO_MOVE(FrameGraphBuilder);

        FGImageHandle create_image(const ImageDesc &desc);
        FGImageHandle read(FGImageHandle image, ReadUsage usage = ReadUsage::Sampled);
        FGImageHandle write(FGImageHandle image, WriteUsage usage = WriteUsage::ColorAttachment);

        FGBufferHandle create_buffer(const BufferDesc &desc);
        FGBufferHandle read(FGBufferHandle buffer, BufferReadUsage usage = BufferReadUsage::Vertex);
        FGBufferHandle write(FGBufferHandle   buffer,
                             BufferWriteUsage usage = BufferWriteUsage::Storage);

      private:
        friend class FrameGraph;

        u32                                    m_pass_index = UINT32_MAX;
        std::pmr::vector<FGImageReadAccess>   *m_image_reads = nullptr;
        std::pmr::vector<FGImageWriteAccess>  *m_image_writes = nullptr;
        std::pmr::vector<FGImageEntry>        *m_images = nullptr;
        u32                                   *m_next_image_index = nullptr;
        std::pmr::vector<FGBufferEntry>       *m_buffers = nullptr;
        u32                                   *m_next_buffer_index = nullptr;
        std::pmr::vector<FGBufferReadAccess>  *m_buffer_reads = nullptr;
        std::pmr::vector<FGBufferWriteAccess> *m_buffer_writes = nullptr;
    };

    class FGPassContext final {
      public:
        FGPassContext() = default;
        ~FGPassContext() = default;

        MANTLE_NO_COPY_NO_MOVE(FGPassContext);

        void bind_pipeline(GraphicsPipelineHandle pipeline);
        void bind_pipeline(ComputePipelineHandle pipeline);

        void begin_rendering(const FGRenderingInfo &info);
        void end_rendering();

        void set_viewport(f32 x, f32 y, f32 width, f32 height);
        void set_scissor(i32 x, i32 y, u32 width, u32 height);

        void bind_vertex_buffer(FGBufferHandle buffer, u32 binding, usize offset = 0);
        void bind_index_buffer(FGBufferHandle buffer, usize offset = 0);

        void draw(const FGDrawInfo &info);
        void draw_indexed(const FGDrawIndexedInfo &info);
        void draw_indirect(const FGDrawIndirectInfo &info);
        void draw_indexed_indirect(const FGDrawIndexedIndirectInfo &info);
        void dispatch(const FGDispatchInfo &info);
        void dispatch_indirect(const FGDispatchIndirectInfo &info);

        void fill_buffer(const FGFillBufferInfo &info);
        void copy_buffer(const FGBufferCopyInfo &info);
        void copy_buffer_to_image(const FGBufferImageCopyInfo &info);
        void copy_image_to_buffer(const FGImageBufferCopyInfo &info);
        void copy_image(const FGImageCopyInfo &info);
        void blit_image(const FGImageBlitInfo &info);

        void clear_color_image(FGImageHandle image, f32 r, f32 g, f32 b, f32 a);
        void clear_depth_image(FGImageHandle image, f32 depth);

        u32 get_bindless_index(FGImageHandle handle, BindlessImageType type);
        u32 get_bindless_index(FGBufferHandle handle);

        void push_constants(const void *data, ShaderStage stage);

      private:
        struct Impl;

        friend class FrameGraph;
        friend class Renderer;
        void init(Impl *impl);

        Impl *m_impl;
    };

    template <typename TData>
    concept CRenderPassData = std::is_default_constructible_v<TData>;

    template <typename TData, typename TSetup>
    concept CRenderPassSetupLambda = requires(TSetup fn, FrameGraphBuilder &builder, TData &data) {
        { fn(builder, data) } -> std::same_as<void>;
    };

    template <typename TData, typename TExecute>
    concept CRenderPassExecuteLambda =
        requires(TExecute fn, const TData &data, FGPassContext &ctx) {
            { fn(ctx, data) } -> std::same_as<void>;
        };

    class FrameGraph final {
        friend class Renderer;

      public:
        explicit FrameGraph(ArenaAllocator *arena);

        template <typename TData, typename TSetup, typename TExecute>
            requires CRenderPassData<TData> && CRenderPassSetupLambda<TData, TSetup> &&
                     CRenderPassExecuteLambda<TData, TExecute>
        const TData &add_pass(std::string_view name, TSetup &&setup, TExecute &&execute) {
            static_assert(std::is_trivially_destructible_v<std::decay_t<TExecute>>,
                          "Render pass lambdas must be trivially destructible.");
            struct Combined {
                TData data;
                alignas(16) std::decay_t<TExecute> exec;
            };

            auto *combined =
                static_cast<Combined *>(m_arena->push(sizeof(Combined), alignof(Combined)));
            new (&combined->data) TData {};
            new (&combined->exec) std::decay_t<TExecute>(std::forward<TExecute>(execute));

            FrameGraphBuilder builder;
            builder.m_pass_index = static_cast<u32>(m_passes.size());
            builder.m_image_reads = &m_image_reads;
            builder.m_image_writes = &m_image_writes;
            builder.m_images = &m_images;
            builder.m_next_image_index = &m_next_image_index;
            builder.m_buffers = &m_buffers;
            builder.m_next_buffer_index = &m_next_buffer_index;
            builder.m_buffer_reads = &m_buffer_reads;
            builder.m_buffer_writes = &m_buffer_writes;
            setup(builder, combined->data);

            m_passes.push_back({
                .name = name,
                .execute_data = combined,
                .execute_fn =
                    [](void *d, FGPassContext &ctx) {
                        auto *c = static_cast<Combined *>(d);
                        c->exec(ctx, c->data);
                    },
            });

            return combined->data;
        }

        FGImageHandle  import_image(ImageHandle image);
        FGBufferHandle import_buffer(BufferHandle buffer);

        Blackboard &blackboard() { return m_blackboard; }

      private:
        struct RenderPassNode {
            std::string_view name;
            void            *execute_data = nullptr;
            void (*execute_fn)(void *data, FGPassContext &ctx) = nullptr;
        };


        ArenaAllocator                  *m_arena = nullptr;
        ScopeArena                       m_scope;
        ArenaResource                    m_resource {};
        Blackboard                       m_blackboard;
        std::pmr::vector<RenderPassNode> m_passes;

        std::pmr::vector<FGImageEntry>  m_images;
        std::pmr::vector<FGBufferEntry> m_buffers;
        u32                             m_next_image_index = 0;
        u32                             m_next_buffer_index = 0;

        std::pmr::vector<FGImageReadAccess>  m_image_reads;
        std::pmr::vector<FGImageWriteAccess> m_image_writes;

        std::pmr::vector<FGBufferReadAccess>  m_buffer_reads;
        std::pmr::vector<FGBufferWriteAccess> m_buffer_writes;
    };
} // namespace mantle
