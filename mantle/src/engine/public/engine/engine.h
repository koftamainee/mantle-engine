#pragma once
#include "math/aabb.h"
#include "math/frustum.h"
#include "camera/camera.h"
#include "renderer/renderer.h"
#include "window/window.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <queue>
#include <unordered_map>

#include "world/chunk_storage.h"

namespace mantle {
    class Engine final {
      public:
        void init();
        void run();
        void destroy();

      private:
        void update(f32 delta_time);
        void render();

      private:
        bool m_is_initialized = false;
        Window m_window{};
        Renderer m_renderer{};
        Camera m_camera{};

        ChunkStorage m_chunk_storage;

        struct ChunkRenderData {
            MeshHandle mesh{};
            glm::mat4 model{};
            AABB aabb{};
        };

        std::unordered_map<glm::ivec3, ChunkRenderData> m_chunk_render_data{};
        std::queue<glm::ivec3> m_dirty_chunks{};

        Frustum m_frustum{};

        f32 m_last_time = 0.0f;

        f32 m_mouse_x = 0.0f;
        f32 m_mouse_y = 0.0f;

        f32 m_camera_speed = m_base_camera_speed;

        static constexpr f32 m_base_camera_speed = 100.0f;
        static constexpr f32 m_mouse_sensitivity = 0.5f;
    };
} // namespace mantle
