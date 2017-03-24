#include "memory_pool.h"
#include "log.h"
#include "ne_assert.h"
#include "ne_mem.h"

ne_error_code ne_memory_pool_init(ne_memory_pool_t *pool, size_t block_size,
                                  size_t block_count) {
  // overflow is not checked, make sure block_size * block_count will not
  // overflow
  NEMEMSET(pool, 1);

  pool->pool_size = block_count * block_size;
  pool->pool_data = calloc(pool->pool_size, 1);
  if (!pool->pool_data) {
    NELOG(NELOG_ERROR, "No enough memory for memory pool.");
    return NE_ENOMEM;
  }

  pool->block_count = block_count;
  pool->block_size = block_size;

  pool->bufs = NEALLOC(ne_memory_buf_t, pool->block_count);
  if (!pool->bufs) {
    NELOG(NELOG_ERROR, "No enough memory for memory pool bufs.");
    return NE_ENOMEM;
  }

  SLIST_INIT(&pool->unused_list);

  for (size_t i = 0; i < block_count; ++i) {
    ne_memory_buf_t *buf = pool->bufs + i;
    buf->data = (char *)pool->pool_data + i * block_size;
    buf->size = block_size;
    buf->type = POOL;
    buf->pool = pool;

    SLIST_INSERT_HEAD(&pool->unused_list, buf, pointer);
  }

  NELOG(NELOG_DEBUG, "Successfully initialized memory pool.");

  return 0;
}

static ne_memory_buf_t *__memory_buf_create(size_t size,
                                            ne_memory_pool_t *pool) {
  ne_memory_buf_t *buf = (ne_memory_buf_t *)calloc(sizeof(ne_memory_buf_t), 1);
  if (!buf) {
    NELOG(NELOG_ERROR, "No enough memory for new memory buf.");
    return NULL;
  }

  buf->data = calloc(size, 1);
  if (!buf->data) {
    NELOG(NELOG_ERROR, "No enough memory for new memory buf.");
    return NULL;
  }

  buf->type = ALLOC;
  buf->size = size;
  buf->pool = pool;
  buf->used = true;

  pool->alloc_bufs++;

  NELOG(NELOG_DEBUG, "Successfully created new memory buf.");

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

  NEASSERT(buf->type == POOL);

  buf->used = false;
  SLIST_INSERT_HEAD(&buf->pool->unused_list, buf, pointer);
}

static bool __memory_pool_used(ne_memory_pool_t *pool) {
  if (pool->alloc_bufs)
    return true;

  for (size_t i = 0; i < pool->block_count; ++i) {
    if (pool->bufs[i].used)
      return true;
  }
  return false;
}

bool ne_memory_pool_free(ne_memory_pool_t *pool) {
  if (__memory_pool_used(pool)) {
    NELOG(NELOG_WARNING,
          "Failed to free memory pool, some bufs are still in use.");
    return false;
  }

  free(pool->pool_data);
  free(pool->bufs);
  free(pool);

  NELOG(NELOG_DEBUG, "Successfully freed memory pool.");

  return true;
}
