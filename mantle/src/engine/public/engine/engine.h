#pragma once
#include "camera/camera.h"
#include "renderer/renderer.h"
#include "window/window.h"
#include "world/world.h"

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
        World m_world{};
        Camera m_camera{};

        std::vector<MeshHandle> m_meshes{};
        std::vector<glm::mat4> m_models{};

        f32 m_last_time = 0.0f;

        f32 m_mouse_x = 0.0f;
        f32 m_mouse_y = 0.0f;

        static constexpr f32 m_camera_speed = 100.0f;
        static constexpr f32 m_mouse_sensitivity = 0.5f;
    };
} // namespace mantle
