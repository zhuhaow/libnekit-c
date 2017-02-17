#ifndef ERROR_H
#define ERROR_H

#include "types.h"

typedef int ne_error_code;

#define NE_NOERR 0
#define NE_ENOMEM -1

typedef struct {
  ne_error_code error_code;

  /* private */
  char *error_str;
  bool own_str;
} ne_error_t;

void ne_error_init(ne_error_t *error);
ne_error_t *ne_error_create(ne_error_code error_code);
char *ne_error_str(ne_error_t *error);
void ne_error_free(ne_error_t *error);

#endif /* ERROR_H */
