#include "memory_pool.h"
#include "mem.h"
#include <assert.h>

int ne_memory_pool_init(ne_memory_pool_t *pool, size_t block_size,
                     size_t block_count) {
  // overflow is not checked, make sure block_size * block_count will not
  // overflow
  pool->pool_size = block_count * block_size;
  pool->pool_data = calloc(pool->pool_size, 1);
  if (!pool->pool_data)
    return -1;

  pool->block_count = block_count;
  pool->block_size = block_size;

  pool->bufs = ALLOC(ne_memory_buf_t, pool->block_count);
  if (!pool->bufs)
    return -1;

  SLIST_INIT(&pool->unused_list);

  for (int i = 0; i < block_count; ++i) {
    ne_memory_buf_t *buf = pool->bufs + i;
    buf->data = pool->pool_data + i * block_size;
    buf->size = block_size;
    buf->type = POOL;
    buf->pool = pool;

    SLIST_INSERT_HEAD(&pool->unused_list, buf, pointer);
  }

  return 0;
}

ne_memory_buf_t *ne_memory_buf_create(size_t size) {
  ne_memory_buf_t *buf = (ne_memory_buf_t *)malloc(sizeof(ne_memory_buf_t));
  buf->data = malloc(size);
  buf->type = ALLOC;
  buf->size = size;
  buf->pool = NULL;
  return buf;
}

ne_memory_buf_t *memory_pool_get_buf(ne_memory_pool_t *pool, bool alloc_if_full) {
  if (pool && !SLIST_EMPTY(&pool->unused_list)) {
    ne_memory_buf_t *buf = SLIST_FIRST(&pool->unused_list);
    buf->used = true;
    return buf;
  }

  return ne_memory_buf_create(pool->block_size);
}

void ne_memory_buf_free(ne_memory_buf_t *buf) {
  if (buf->type == ALLOC) {
    free(buf->data);
    free(buf);
  }

  assert(buf->type == POOL);

  buf->used = false;
  SLIST_INSERT_HEAD(&buf->pool->unused_list, buf, pointer);
}

bool _memory_pool_used(ne_memory_pool_t *pool) {
  for (int i = 0; i < pool->block_count; ++i) {
    if (pool->bufs[i].used)
      return true;
  }
  return false;
}

bool ne_memory_pool_free(ne_memory_pool_t *pool) {
  if (_memory_pool_used(pool))
    return false;

  free(pool->pool_data);
  free(pool->bufs);
  free(pool);
  return true;
}
