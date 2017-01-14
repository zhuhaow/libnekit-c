#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include "types.h"
#include <stddef.h>
#include <stdint.h>

enum memory_t { POOL, ALLOC };

typedef struct memeory_pool memory_pool_t;
typedef struct memory_buf memory_buf_t;

struct memory_buf {
  enum memory_t type;
  bool used;
  void *data;
  size_t size;
  memory_pool_t *pool;
  // create a linked list in the buf itself.
  // used for unused_list only.
  memory_buf_t *next;
};

struct memeory_pool {
  void *pool_data;
  size_t pool_size;
  size_t block_count;
  size_t block_size;
  memory_buf_t *bufs;
  memory_buf_t *unused_bufs;
};

int memory_pool_init(memory_pool_t *pool, size_t block_size, size_t block_count);
memory_buf_t *memory_buf_create(size_t size);
memory_buf_t *memory_pool_get_buf(memory_pool_t* pool, bool alloc_if_full);
void memory_buf_free(memory_buf_t *buf);
bool memory_pool_free(memory_pool_t *pool);

#endif
