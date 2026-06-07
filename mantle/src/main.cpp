#include "core/logger.h"
#include "engine/engine.h"

#define SOL_USE_STD_OPTIONAL 1
#include <sol/sol.hpp>

int main() {
    mantle::init_logger();

    sol::state lua;
    lua.open_libraries();
    lua.script("print('Hello from LuaJIT')");

    mantle::Engine engine;
    engine.init();
    engine.run();
    engine.destroy();

    mantle::raw_logger()->info("see you soon~\n");

    return 0;
}
