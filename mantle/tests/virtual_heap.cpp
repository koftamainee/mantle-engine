#include "core/memory/os_memory.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"
#include <gtest/gtest.h>

using namespace mantle;

class VirtualHeapTest : public ::testing::Test {
protected:
    OSMemory m_os;
    VirtualHeap m_heap;
    static constexpr usize RESERVE_SIZE = 1024 * 1024;

    void SetUp() override {
        m_os.init();
        m_heap.init(m_os, RESERVE_SIZE);
    }

    void TearDown() override {
        m_heap.destroy();
        m_os.destroy();
    }
};

TEST_F(VirtualHeapTest, initial_state) {
    EXPECT_EQ(m_heap.reserved(), RESERVE_SIZE);
    EXPECT_EQ(m_heap.used(), 0u);
    EXPECT_EQ(m_heap.committed(), 0u);
}

TEST_F(VirtualHeapTest, take_increases_used) {
    MemoryBlock block = m_heap.take(64);
    EXPECT_NE(block.ptr, nullptr);
    EXPECT_EQ(block.size, 64u);
    EXPECT_GE(m_heap.used(), 64u);
}

TEST_F(VirtualHeapTest, take_commits_pages) {
    m_heap.take(1);
    usize page = m_os.page_size();
    EXPECT_EQ(m_heap.committed(), page);
}

TEST_F(VirtualHeapTest, take_multiple_increases_used) {
    m_heap.take(100);
    usize after_first = m_heap.used();
    m_heap.take(200);
    EXPECT_GT(m_heap.used(), after_first);
}

TEST_F(VirtualHeapTest, reserved_unchanged_after_take) {
    m_heap.take(100);
    m_heap.take(200);
    EXPECT_EQ(m_heap.reserved(), RESERVE_SIZE);
}

TEST_F(VirtualHeapTest, take_large_rounds_committed_to_page) {
    usize page = m_os.page_size();
    m_heap.take(page);
    EXPECT_EQ(m_heap.committed(), page);
}

TEST_F(VirtualHeapTest, heap_destroy_reinit) {
    m_heap.destroy();
    m_heap.init(m_os, RESERVE_SIZE);
    EXPECT_EQ(m_heap.reserved(), RESERVE_SIZE);
    EXPECT_EQ(m_heap.used(), 0u);
}

TEST_F(VirtualHeapTest, take_exact_page_multiple) {
    usize page = m_os.page_size();
    m_heap.take(page * 2);
    EXPECT_EQ(m_heap.committed(), page * 2);
}

TEST_F(VirtualHeapTest, take_memory_writable) {
    MemoryBlock block = m_heap.take(256);
    auto *data = static_cast<u8 *>(block.ptr);
    for (usize i = 0; i < 256; i++) {
        data[i] = static_cast<u8>(i);
    }
    for (usize i = 0; i < 256; i++) {
        EXPECT_EQ(data[i], static_cast<u8>(i));
    }
}
