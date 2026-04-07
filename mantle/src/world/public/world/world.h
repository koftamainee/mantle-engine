#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include "chunk.h"

namespace mantle {
    class World final {
      public:
        World() = default;
        ~World();

        World(const World &) = delete;
        World &operator=(const World &) = delete;
        World(World &&) noexcept = delete;
        World &operator=(World &&) noexcept = delete;

        void init();
        void destroy();

        Chunk &get_or_create_chunk(glm::ivec3 chunk_coords);
        Chunk *get_chunk(glm::ivec3 chunk_coords);
        Chunk *generate_chunk(glm::ivec3 chunk_coords);

        void for_each_chunk(const std::function<void(Chunk &)>&fn);

      private:
        bool m_is_initialized = false;
        std::vector<Chunk> m_chunk_pool{};
        std::unordered_map<glm::ivec3, u32> m_chunk_index{};
    };
} // namespace mantle
