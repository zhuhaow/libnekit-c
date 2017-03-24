#ifndef NE_ASSERT_H
#define NE_ASSERT_H


#ifndef NDEBUG

#include <stdio.h>
#include <stdlib.h>

static inline int default_handler(const char *expr, const char *file, int line,
                           const char *msg) {
  fprintf(stderr, "Assertion failed on %s line %d: %s\n", file, line, expr);
  if (msg != NULL)
    fprintf(stderr, "Diagnostic: %s\n", msg);
  return 1;
}

#define _NE_ASSERT_HANDLER(x, y, z, t) (default_handler(x, y, z, t))

#define NE_XASSERT(x, m)                                                       \
  (!(x) && _NE_ASSERT_HANDLER(#x, __FILE__, __LINE__, m) && (abort(), 1))

#else
#define NE_XASSERT(x, m) (x)
#endif

/* This is guaranteed to run but will be checked the same as assert(); */
#define NEASSERTE(x) NE_XASSERT(x, NULL)

#include <assert.h>
/* The same as the assert() provided in the standard library. */
#define NEASSERT(x) assert(x);

#endif /* NE_ASSERT_H */
