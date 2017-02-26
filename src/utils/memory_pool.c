#include "memory_pool.h"
#include "mem.h"
#include <assert.h>

ne_error_code ne_memory_pool_init(ne_memory_pool_t *pool, size_t block_size,
                     size_t block_count) {
  // overflow is not checked, make sure block_size * block_count will not
  // overflow
  MEMSET(pool, 1);

  pool->pool_size = block_count * block_size;
  pool->pool_data = calloc(pool->pool_size, 1);
  if (!pool->pool_data)
    return NE_ENOMEM;

  pool->block_count = block_count;
  pool->block_size = block_size;

  pool->bufs = ALLOC(ne_memory_buf_t, pool->block_count);
  if (!pool->bufs)
    return NE_ENOMEM;

  SLIST_INIT(&pool->unused_list);

  for (int i = 0; i < block_count; ++i) {
    ne_memory_buf_t *buf = pool->bufs + i;
    buf->data = (char *)pool->pool_data + i * block_size;
    buf->size = block_size;
    buf->type = POOL;
    buf->pool = pool;

    SLIST_INSERT_HEAD(&pool->unused_list, buf, pointer);
  }

  return 0;
}

ne_memory_buf_t *__memory_buf_create(size_t size, ne_memory_pool_t *pool) {
  ne_memory_buf_t *buf = (ne_memory_buf_t *)calloc(sizeof(ne_memory_buf_t), 1);
  if (!buf)
    return NULL;

  buf->data = calloc(size, 1);
  if (!buf->data)
    return NULL;

  buf->type = ALLOC;
  buf->size = size;
  buf->pool = pool;
  buf->used = true;

  pool->alloc_bufs++;

  return buf;
}

ne_memory_buf_t *ne_memory_pool_get_buf(ne_memory_pool_t *pool) {
  if (!SLIST_EMPTY(&pool->unused_list)) {
    ne_memory_buf_t *buf = SLIST_FIRST(&pool->unused_list);
    SLIST_REMOVE_HEAD(&pool->unused_list, pointer);
    buf->used = true;
    return buf;
  }

  return __memory_buf_create(pool->block_size, pool);
}

void ne_memory_buf_free(ne_memory_buf_t *buf) {
  if (buf->type == ALLOC) {
    free(buf->data);
    buf->pool->alloc_bufs--;
    free(buf);
    return;
  }

  assert(buf->type == POOL);

  buf->used = false;
  SLIST_INSERT_HEAD(&buf->pool->unused_list, buf, pointer);
}

bool __memory_pool_used(ne_memory_pool_t *pool) {
  if (pool->alloc_bufs)
    return true;

  for (int i = 0; i < pool->block_count; ++i) {
    if (pool->bufs[i].used)
      return true;
  }
  return false;
}

bool ne_memory_pool_free(ne_memory_pool_t *pool) {
  if (__memory_pool_used(pool))
    return false;

  free(pool->pool_data);
  free(pool->bufs);
  free(pool);
  return true;
}
