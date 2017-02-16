#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include "types.h"
#include <stdint.h>

#include "queue.h"
enum ne_memory_t { POOL, ALLOC };

typedef struct ne_memeory_pool ne_memory_pool_t;
typedef struct ne_memory_buf ne_memory_buf_t;

struct ne_memory_buf {
  enum ne_memory_t type;
  bool used;
  void *data;
  size_t size;
  ne_memory_pool_t *pool;
  SLIST_ENTRY(ne_memory_buf) pointer;
};

struct ne_memeory_pool {
  void *pool_data;
  size_t pool_size;
  size_t block_count;
  size_t block_size;
  ne_memory_buf_t *bufs;
  SLIST_HEAD(memory_buf_list, ne_memory_buf) unused_list;
};

int ne_memory_pool_init(ne_memory_pool_t *pool, size_t block_size,
                     size_t block_count);
ne_memory_buf_t *memory_buf_create(size_t size);
ne_memory_buf_t *memory_pool_get_buf(ne_memory_pool_t *pool, bool alloc_if_full);
void memory_buf_free(ne_memory_buf_t *buf);
bool memory_pool_free(ne_memory_pool_t *pool);

#endif
