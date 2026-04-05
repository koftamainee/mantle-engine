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
            NeedsResize,
        };

        Renderer();
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) noexcept = delete;
        Renderer &operator=(Renderer &&) noexcept = delete;

        void init(const Window &window);
        void destroy();

        Result begin_frame() const;
        Result end_frame() const;

        void begin_pass() const;
        void end_pass() const;

        void draw_mesh(MeshHandle handle) const;

        void resize(uint32_t width, uint32_t height) const;

        GPUResourceManager& get_resource_manager();

    private:
        bool m_is_initialized = false;

        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace mantle