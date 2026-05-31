#include "bgm.h"
#include "core/memory/arena_allocator.h"
#include "core/memory/memory_block.h"
#include "core/types.h"
#include "world/chunk.h"
#include "world/chunk_mesh_data.h"
#include "world/face_mask.h"

#include <gtest/gtest.h>

using namespace mantle;

static u8 g_scratch[24 * 1024 * 1024];

struct TestScratch {
    ArenaAllocator arena;

    TestScratch() {
        arena.init({g_scratch, sizeof(g_scratch)});
    }
};

static Chunk make_full_chunk(Voxel v) {
    Chunk c{};
    for (auto &vox : c.voxels) vox = v;
    return c;
}

TEST(BGM, EmptyChunkProducesNoQuads) {
    TestScratch s;
    Chunk chunk = {};

    const Chunk *neighbors[6] = {};
    auto mesh = mantle::bgm::mesh_chunk(chunk, neighbors, {0, 0, 0}, s.arena);

    EXPECT_EQ(mesh.quad_count, 0u);
    EXPECT_TRUE(mesh.empty());
}

TEST(BGM, SingleVoxelProducesSixQuads) {
    TestScratch s;
    Chunk chunk = {};
    chunk.voxels[0] = 1;

    const Chunk *neighbors[6] = {};
    auto mesh = mantle::bgm::mesh_chunk(chunk, neighbors, {0, 0, 0}, s.arena);

    EXPECT_EQ(mesh.quad_count, 6u);
    EXPECT_FALSE(mesh.empty());
}

TEST(BGM, FullChunkWithNoNeighbors) {
    TestScratch s;
    Chunk chunk = make_full_chunk(1);

    const Chunk *neighbors[6] = {};
    auto mesh = mantle::bgm::mesh_chunk(chunk, neighbors, {0, 0, 0}, s.arena);

    EXPECT_EQ(mesh.quad_count, 6u);
}

TEST(BGM, FullChunkWithFullNeighbors) {
    TestScratch s;
    Chunk chunk = make_full_chunk(1);
    Chunk nb = make_full_chunk(1);

    const Chunk *neighbors[6] = {&nb, &nb, &nb, &nb, &nb, &nb};
    auto mesh = mantle::bgm::mesh_chunk(chunk, neighbors, {0, 0, 0}, s.arena);

    EXPECT_EQ(mesh.quad_count, 0u);
}

TEST(BGM, FlatLayer) {
    TestScratch s;
    Chunk chunk = {};
    for (u32 x = 0; x < 32; x++)
        for (u32 z = 0; z < 32; z++)
            chunk.voxels[x + 0 * 32 + z * 1024] = 1;

    const Chunk *neighbors[6] = {};
    auto mesh = mantle::bgm::mesh_chunk(chunk, neighbors, {0, 0, 0}, s.arena);

    EXPECT_EQ(mesh.quad_count, 6u);
}

TEST(BGM, Column) {
    TestScratch s;
    Chunk chunk = {};
    for (u32 y = 0; y < 32; y++)
        chunk.voxels[0 + y * 32 + 0 * 1024] = 1;

    const Chunk *neighbors[6] = {};
    auto mesh = mantle::bgm::mesh_chunk(chunk, neighbors, {0, 0, 0}, s.arena);

    EXPECT_EQ(mesh.quad_count, 6u);
}

TEST(BGM, OneMissing) {
    TestScratch s;
    Chunk chunk = make_full_chunk(1);
    chunk.voxels[16 + 16 * 32 + 16 * 1024] = 0;

    const Chunk *neighbors[6] = {};
    auto mesh = mantle::bgm::mesh_chunk(chunk, neighbors, {0, 0, 0}, s.arena);

    EXPECT_EQ(mesh.quad_count, 12u);
}

TEST(BGM, VertexFormat) {
    TestScratch s;
    Chunk chunk = {};
    chunk.voxels[0] = 1;

    const Chunk *neighbors[6] = {};
    auto mesh = mantle::bgm::mesh_chunk(chunk, neighbors, {0, 0, 0}, s.arena);

    ASSERT_EQ(mesh.quad_count, 6u);
    ASSERT_EQ(mesh.vertex_count(), 24u);

    u32 packed = mesh.vertices[0].packed;
    EXPECT_EQ(packed & 0x7, 0u);               // face_index = 0 (axis=0, pos=0)
    EXPECT_EQ((packed >> 3) & 0x3, 0u);         // AO = 0
    EXPECT_EQ((packed >> 8) & 0xFFFF, 1u);      // material = 1

    EXPECT_FLOAT_EQ(mesh.vertices[0].x, 0.0f);
    EXPECT_FLOAT_EQ(mesh.vertices[0].y, 0.0f);
    EXPECT_FLOAT_EQ(mesh.vertices[0].z, 0.0f);
}
