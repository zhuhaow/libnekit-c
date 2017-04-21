#ifndef NE_LOG_H
#define NE_LOG_H

#include "config.h"

#define NELOG_DEBUG 0
#define NELOG_INFO 1
#define NELOG_WARNING 2
#define NELOG_ERROR 3

#define NELOG_DEBUG_STR "DEBUG"
#define NELOG_INFO_STR "INFO"
#define NELOG_WARNING_STR "WARNING"
#define NELOG_ERROR_STR "ERROR"

void ne_log_set_level(int level);
int ne_log_get_level();

#ifndef NELOG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define NELOG(LEVEL, FMT, ...)                                                 \
  do {                                                                         \
    printf("%s:%s:%d: " FMT "\n", LEVEL##_STR, __FILE__, __LINE__,             \
           ##__VA_ARGS__);                                                     \
    fflush(stdout);                                                   \
  } while (0)
#pragma clang diagnostic pop
#endif

#endif /* NE_LOG_H */
