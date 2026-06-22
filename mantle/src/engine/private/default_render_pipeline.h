// Copyright 2026 Mantle
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <glm/glm.hpp>

#include <vector>

#include "mantle/assets/asset_manager.h"
#include "mantle/core/macros.h"
#include "mantle/renderer/render_pipeline.h"
#include "mantle/renderer/types.h"

namespace spdlog {
    class logger;
}

namespace flecs {
    class world;
}

namespace mantle {

    class Renderer;

    static constexpr u32 MAX_DIRECTIONAL_LIGHTS = 8;

    struct DirectionalLight {
        glm::vec4 direction;
        glm::vec4 color;
    };

    struct GPULightData {
        glm::vec4        camera_pos;
        glm::vec4        ambient;
        DirectionalLight lights[MAX_DIRECTIONAL_LIGHTS];
        u32              light_count;
        u32              _padding[3];
    };
    static_assert(sizeof(GPULightData) == 304);

    class DefaultRenderPipeline final : public RenderPipeline {
      public:
        DefaultRenderPipeline(flecs::world &world, AssetManager &assets, Renderer &renderer);
        ~DefaultRenderPipeline() override;

        MANTLE_NO_COPY_NO_MOVE(DefaultRenderPipeline);

        void add_passes(FrameGraph &graph) override;
        void rebuild_materials();

        void set_ambient(glm::vec3 color, f32 intensity);
        u32  add_directional_light(glm::vec3 direction, glm::vec3 color, f32 intensity);
        void clear_lights();

      private:
        void create_pipeline(Renderer &renderer);

        flecs::world          &m_world;
        AssetManager          &m_assets;
        Renderer              &m_renderer;
        std::string            m_asset_base_path;
        GraphicsPipelineHandle m_pbr_pipeline {};

        u32 m_material_buffer_idx = UINT32_MAX;
        u32 m_frame_data_buffer_idx = UINT32_MAX;
        u32 m_light_buffer_idx = UINT32_MAX;

        BufferHandle m_material_buffer {};
        BufferHandle m_frame_data_buffer {};
        BufferHandle m_light_buffer {};

        GPULightData m_light_data {};

        std::vector<TextureUpload> m_pending_uploads;

        spdlog::logger *m_logger = nullptr;
    };

} // namespace mantle
