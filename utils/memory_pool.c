#include <stdlib.h>
#include <assert.h>
#include "memory_pool.h"

int memory_pool_init(memory_pool_t *pool, size_t block_size, size_t block_count) {
  // overflow is not checked, make sure block_size * block_count will not overflow
  pool->pool_size = block_count * block_size;
  pool->pool_data = malloc(pool->pool_size);
  if (!pool->pool_data)
    return -1;

  pool->block_count = block_count;
  pool->block_size = block_size;

  pool->unused_bufs = NULL;

  pool->bufs = calloc(pool->block_count, sizeof(memory_buf_t));
  if (!pool->bufs)
    return -1;
  for (int i = 0; i < block_count; ++i) {
    memory_buf_t *buf = pool->bufs + i;
    buf->data = pool->pool_data + i * block_size;
    buf->size = block_size;
    buf->type = POOL;
    buf->pool = pool;

    buf->next = pool->unused_bufs;
    pool->unused_bufs = buf;
  }

  return 0;
}

memory_buf_t *memory_buf_create(size_t size) {
  memory_buf_t *buf = (memory_buf_t *)malloc(sizeof(memory_buf_t));
  buf->data = malloc(size);
  buf->type = ALLOC;
  buf->size = size;
  buf->next = NULL;
  buf->pool = NULL;
  return buf;
}

memory_buf_t *memory_pool_get_buf(memory_pool_t* pool, bool alloc_if_full) {
  if (pool->unused_bufs) {
    memory_buf_t *buf = pool->unused_bufs;
    pool->unused_bufs = buf->next;
    buf->used = true;
    buf->next = NULL;
    return buf;
  }

  return memory_buf_create(pool->block_size);
}

void memory_buf_free(memory_buf_t *buf) {
  if (buf->type == ALLOC) {
    free(buf->data);
    free(buf);
  }

  assert(buf->type == POOL);

  buf->used = false;
  buf->next = buf->pool->unused_bufs;
  buf->pool->unused_bufs = buf;
}

bool _memory_pool_used(memory_pool_t *pool) {
  for (int i = 0; i < pool->block_count; ++i) {
    if (pool->bufs[i].used)
      return true;
  }
  return false;
}

bool memory_pool_free(memory_pool_t *pool) {
  if (_memory_pool_used(pool))
    return false;

  free(pool->pool_data);
  free(pool->bufs);
  free(pool);
  return true;
}
