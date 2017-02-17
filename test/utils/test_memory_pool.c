#include "check.h"
#include "memory_pool.h"
#include "error.h"
#include "queue.h"
#include "mem.h"

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 64

START_TEST (memory_pool_init) {
  ne_memory_pool_t *pool = ALLOC(ne_memory_pool_t, 1);
  ck_assert(ne_memory_pool_init(pool, BLOCK_SIZE, BLOCK_COUNT) == NE_NOERR);
  ck_assert_uint_eq(pool->pool_size, BLOCK_COUNT * BLOCK_SIZE);
  ck_assert_uint_eq(pool->block_size, BLOCK_SIZE);
  ck_assert_uint_eq(pool->block_count, BLOCK_COUNT);
}
END_TEST

START_TEST (memory_pool_buf) {
  ne_memory_pool_t *pool = ALLOC(ne_memory_pool_t, 1);
  ck_assert(ne_memory_pool_init(pool, BLOCK_SIZE, BLOCK_COUNT) == NE_NOERR);

  ne_memory_buf_t *pool_bufs[BLOCK_COUNT];
  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ck_assert_ptr_nonnull(pool_bufs[i] = ne_memory_pool_get_buf(pool));
    ck_assert(pool_bufs[i]->used);
    ck_assert(pool_bufs[i]->size == BLOCK_SIZE);
    ck_assert_ptr_nonnull(pool_bufs[i]->data);
    ck_assert(pool_bufs[i]->type == POOL);
    ck_assert_ptr_eq(pool_bufs[i]->pool, pool);
  }

  ck_assert(!ne_memory_pool_free(pool));

  ne_memory_buf_t *alloc_bufs[BLOCK_COUNT];
  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ck_assert_ptr_nonnull(alloc_bufs[i] = ne_memory_pool_get_buf(pool));
    ck_assert(alloc_bufs[i]->used);
    ck_assert(alloc_bufs[i]->size == BLOCK_SIZE);
    ck_assert_ptr_nonnull(alloc_bufs[i]->data);
    ck_assert(alloc_bufs[i]->type == ALLOC);
    ck_assert_ptr_eq(alloc_bufs[i]->pool, pool);
  }

  ck_assert(!ne_memory_pool_free(pool));

  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ne_memory_buf_free(pool_bufs[i]);
    ne_memory_buf_free(alloc_bufs[i]);
  }

  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ck_assert_ptr_nonnull(pool_bufs[i] = ne_memory_pool_get_buf(pool));
    ck_assert(pool_bufs[i]->used);
    ck_assert(pool_bufs[i]->size == BLOCK_SIZE);
    ck_assert_ptr_nonnull(pool_bufs[i]->data);
    ck_assert(pool_bufs[i]->type == POOL);
    ck_assert_ptr_eq(pool_bufs[i]->pool, pool);
  }

  ck_assert(!ne_memory_pool_free(pool));

  for (int i = 0; i < BLOCK_COUNT; ++i) {
    ne_memory_buf_free(pool_bufs[i]);
  }

  ck_assert(ne_memory_pool_free(pool));
}
END_TEST

Suite *pool_suite() {
  Suite *s;
  TCase *init_case, *buf_case;

  s = suite_create("Memory Pool");

  init_case = tcase_create("Initialize pool");
  tcase_add_test(init_case, memory_pool_init);

  buf_case = tcase_create("Memory buffer");
  tcase_add_test(buf_case, memory_pool_buf);

  suite_add_tcase(s, init_case);
  suite_add_tcase(s, buf_case);

  return s;
}

int main (int argc, char *argv[]) {
  int number_failed;
  Suite *suite = pool_suite();
  SRunner *runner = srunner_create(suite);
  srunner_run_all(runner, CK_NORMAL);
  number_failed = srunner_ntests_failed(runner);
  srunner_free(runner);
  return number_failed;
}
