#include "core/memory/arena_allocator.h"
#include "core/memory/memory_block.h"
#include "core/types.h"
#include <gtest/gtest.h>

using namespace mantle;

static u8 g_scratch[1024 * 1024];

struct ArenaTest : ::testing::Test {
    ArenaAllocator arena;

    void SetUp() override {
        arena.init({g_scratch, sizeof(g_scratch)});
    }

    void TearDown() override {
        arena.destroy();
    }
};

TEST_F(ArenaTest, initially_empty) {
    EXPECT_EQ(arena.offset(), 0u);
    EXPECT_EQ(arena.remaining(), sizeof(g_scratch));
    EXPECT_EQ(arena.size(), sizeof(g_scratch));
}

TEST_F(ArenaTest, push_increases_offset) {
    void *p = arena.push(64);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(arena.offset(), 64u);
    EXPECT_EQ(arena.remaining(), sizeof(g_scratch) - 64);
}

TEST_F(ArenaTest, push_alignment) {
    arena.push(1);
    void *p = arena.push(8, 8);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p) % 8, 0u);
    EXPECT_EQ(arena.offset(), 16u);
}

TEST_F(ArenaTest, push_typed) {
    int *arr = arena.push<int>(10);
    EXPECT_NE(arr, nullptr);
    EXPECT_EQ(arena.offset(), sizeof(int) * 10);
}

TEST_F(ArenaTest, push_zeroed) {
    auto *arr = arena.push_zeroed<u8>(100);
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(arr[i], 0);
    }
    EXPECT_EQ(arena.offset(), 100u);
}

TEST_F(ArenaTest, emplace) {
    struct Point {
        float x, y;
    };
    Point *p = arena.emplace<Point>(3.0f, 4.0f);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->x, 3.0f);
    EXPECT_EQ(p->y, 4.0f);
}

TEST_F(ArenaTest, save_restore) {
    arena.push(100);
    auto marker = arena.save();
    arena.push(200);
    arena.restore(marker);
    EXPECT_EQ(arena.offset(), marker.offset);
}

TEST_F(ArenaTest, save_restore_nested) {
    auto m1 = arena.save();
    arena.push(50);
    auto m2 = arena.save();
    arena.push(30);
    arena.restore(m2);
    EXPECT_EQ(arena.offset(), 50u);
    arena.restore(m1);
    EXPECT_EQ(arena.offset(), 0u);
}

TEST_F(ArenaTest, reset) {
    arena.push(500);
    arena.reset();
    EXPECT_EQ(arena.offset(), 0u);
    EXPECT_EQ(arena.remaining(), sizeof(g_scratch));
}

TEST_F(ArenaTest, multiple_arenas_independent) {
    static u8 buf2[256];
    ArenaAllocator a2;
    a2.init({buf2, sizeof(buf2)});

    arena.push(100);
    a2.push(50);

    EXPECT_EQ(arena.offset(), 100u);
    EXPECT_EQ(a2.offset(), 50u);
    EXPECT_EQ(a2.size(), 256u);

    a2.destroy();
}

TEST_F(ArenaTest, push_large_alignment) {
    alignas(256) static u8 aligned_buf[1024];
    ArenaAllocator aligned_arena;
    aligned_arena.init({aligned_buf, sizeof(aligned_buf)});

    void *p = aligned_arena.push(1, 256);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p) % 256, 0u);

    aligned_arena.destroy();
}
