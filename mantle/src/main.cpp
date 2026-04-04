#include "renderer/public/renderer/renderer.h"
#include "spdlog/spdlog.h"
#include "window/window.h"


int main() {
    spdlog::set_level(spdlog::level::trace);
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

    // while (!window.should_close()) {
    //     window.on_update();
    // }

    renderer.destroy();
    window.destroy();
}
