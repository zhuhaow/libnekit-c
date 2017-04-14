#ifndef NE_MEMORY_POOL_H_
#define NE_MEMORY_POOL_H_

#include "error.h"
#include "queue.h"
#include "types.h"
#include <stdint.h>
#include <stddef.h>

#define UPPER_MEMORY_BUF(ptr) (ne_memory_buf_t *)((uint8_t *)&ptr - offsetof(ne_memory_buf_t, data))

enum ne_memory_t { POOL, ALLOC };

typedef struct ne_memeory_pool ne_memory_pool_t;
typedef struct ne_memory_buf ne_memory_buf_t;

struct ne_memory_buf {
  enum ne_memory_t type;
  bool used;
  uint8_t *data;
  size_t size;
  ne_memory_pool_t *pool;
  SLIST_ENTRY(ne_memory_buf) pointer;
};

struct ne_memeory_pool {
  uint8_t *pool_data;
  size_t pool_size;
  size_t block_count;
  size_t block_size;
  ne_memory_buf_t *bufs;

  size_t alloc_bufs;

  SLIST_HEAD(memory_buf_list, ne_memory_buf) unused_list;
};

ne_err ne_memory_pool_init(ne_memory_pool_t *pool, size_t block_size,
                     size_t block_count);
ne_memory_buf_t *ne_memory_pool_get_buf(ne_memory_pool_t *pool);
void ne_memory_buf_free(ne_memory_buf_t *buf);
bool ne_memory_pool_free(ne_memory_pool_t *pool);

#endif
