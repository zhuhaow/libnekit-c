#include "greatest.h"

#include "error.h"
#include "helper.h"
#include "memory_pool.h"
#include "ne_mem.h"
#include "queue.h"

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 64

TEST memory_pool_init() {
  ne_memory_pool_t *pool = NEALLOC(ne_memory_pool_t, 1);
  ASSERT(ne_memory_pool_init(pool, BLOCK_SIZE, BLOCK_COUNT) == NE_NOERR);
  ASSERT_EQ(pool->pool_size, BLOCK_COUNT * BLOCK_SIZE);
  ASSERT_EQ(pool->block_size, BLOCK_SIZE);
  ASSERT_EQ(pool->block_count, BLOCK_COUNT);
  ne_memory_pool_free(pool);
  PASS();
}

TEST memory_pool_buf() {
  ne_memory_pool_t *pool = NEALLOC(ne_memory_pool_t, 1);
  ASSERT(ne_memory_pool_init(pool, BLOCK_SIZE, BLOCK_COUNT) == NE_NOERR);

  ne_memory_buf_t *pool_bufs[BLOCK_COUNT];
  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ASSERT(pool_bufs[i] = ne_memory_pool_get_buf(pool));
    ASSERT(pool_bufs[i]->used);
    ASSERT(pool_bufs[i]->size == BLOCK_SIZE);
    ASSERT(pool_bufs[i]->data);
    ASSERT(pool_bufs[i]->type == POOL);
    ASSERT_EQ(pool_bufs[i]->pool, pool);
  }

  ASSERT(!ne_memory_pool_free(pool));

  ne_memory_buf_t *alloc_bufs[BLOCK_COUNT];
  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ASSERT(alloc_bufs[i] = ne_memory_pool_get_buf(pool));
    ASSERT(alloc_bufs[i]->used);
    ASSERT(alloc_bufs[i]->size == BLOCK_SIZE);
    ASSERT(alloc_bufs[i]->data);
    ASSERT(alloc_bufs[i]->type == ALLOC);
    ASSERT_EQ(alloc_bufs[i]->pool, pool);
  }

  ASSERT(!ne_memory_pool_free(pool));

  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ne_memory_buf_free(pool_bufs[i]);
    ne_memory_buf_free(alloc_bufs[i]);
  }

  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ASSERT(pool_bufs[i] = ne_memory_pool_get_buf(pool));
    ASSERT(pool_bufs[i]->used);
    ASSERT(pool_bufs[i]->size == BLOCK_SIZE);
    ASSERT(pool_bufs[i]->data);
    ASSERT(pool_bufs[i]->type == POOL);
    ASSERT_EQ(pool_bufs[i]->pool, pool);
  }

  ASSERT(!ne_memory_pool_free(pool));

  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ne_memory_buf_free(pool_bufs[i]);
  }

  ASSERT(ne_memory_pool_free(pool));
  PASS();
}

TEST find_upper_buf() {
  ne_memory_pool_t *pool = NEALLOC(ne_memory_pool_t, 1);
  ASSERT(ne_memory_pool_init(pool, BLOCK_SIZE, BLOCK_COUNT) == NE_NOERR);
  ASSERT_EQ(pool->pool_size, BLOCK_COUNT * BLOCK_SIZE);
  ASSERT_EQ(pool->block_size, BLOCK_SIZE);
  ASSERT_EQ(pool->block_count, BLOCK_COUNT);
  ne_memory_buf_t *buf = ne_memory_pool_get_buf(pool);
  ASSERT_EQ(buf, UPPER_MEMORY_BUF(buf->data));
  ne_memory_buf_free(buf);
  ne_memory_pool_free(pool);
  PASS();
}

SUITE(suite) {
  RUN_TEST(memory_pool_init);
  RUN_TEST(memory_pool_buf);
  RUN_TEST(find_upper_buf);
}

GREATEST_MAIN_DEFS();

TEST_MAIN
