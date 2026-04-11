#pragma once
#include "glm/vec3.hpp"

namespace mantle {
    struct Vertex final {
        glm::vec3 position;
        glm::vec3 normal;
    };
} // namespace mantle
