#include "engine/engine.h"
#include "spdlog/spdlog.h"

int main() {
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    mantle::Engine engine;
    engine.init();
    engine.run();
    engine.destroy();

    return 0;
}
