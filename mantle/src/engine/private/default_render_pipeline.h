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

    class DefaultRenderPipeline final : public RenderPipeline {
      public:
        DefaultRenderPipeline(flecs::world &world, AssetManager &assets, Renderer &renderer);
        ~DefaultRenderPipeline() override;

        MANTLE_NO_COPY_NO_MOVE(DefaultRenderPipeline);

        void add_passes(FrameGraph &graph) override;

      private:
        void create_pipeline(Renderer &renderer);

        flecs::world          &m_world;
        AssetManager          &m_assets;
        std::string            m_asset_base_path;
        GraphicsPipelineHandle m_debug_pipeline {};
        spdlog::logger        *m_logger = nullptr;
    };

} // namespace mantle
