#include "renderer/renderer.h"
#include "spdlog/spdlog.h"
#include "window/window.h"


int main() {
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::warn);
#endif
    mantle::Window window;


    mantle::Window::Properties prop = {
        .title = "Mantle",
        .size = {
            .width = 2560,
            .height = 1600,
        }
    };
    window.init(prop);

    mantle::Renderer renderer;
    renderer.init(window);

    window.set_resize_callback([&](uint32_t w, uint32_t h) {
        renderer.resize(w, h);
    });

    while (!window.should_close()) {
        window.on_update();

        mantle::Renderer::Result result = renderer.begin_frame();
        if (result == mantle::Renderer::Result::NeedsResize) {
            auto [width, height] = window.get_framebuffer_size();
            renderer.resize(width, height);
            continue;
        }
        renderer.begin_pass();
        renderer.draw_triangle();
        renderer.end_pass();

        result = renderer.end_frame();
        if (result == mantle::Renderer::Result::NeedsResize) {
            auto [width, height] = window.get_framebuffer_size();
            renderer.resize(width, height);
        }
    }

    renderer.destroy();
    window.destroy();
}
