#pragma once
#include <memory>
#include <cstdint>
#include "renderer/gpu_resource_manager.h"

namespace mantle {

    class Window;

    class Renderer final {
    public:
        enum class Result {
            Ok,
            FrameNeedsResize,
            InvalidMeshHandle,
        };

        Renderer();
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) noexcept = delete;
        Renderer &operator=(Renderer &&) noexcept = delete;

        void init(const Window &window);
        void destroy();

        void set_camera(const glm::mat4 &view, const glm::mat4 &projection) const;

        Result begin_frame() const;
        Result end_frame() const;

        void begin_pass() const;
        void end_pass() const;

        Result draw_mesh(MeshHandle handle, const glm::mat4 &model) const;

        void resize(uint32_t width, uint32_t height) const;

        GPUResourceManager& get_resource_manager();

    private:
        bool m_is_initialized = false;

        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace mantle