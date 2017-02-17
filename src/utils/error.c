#include <stdlib.h>
#include <string.h>
#include "error.h"

void ne_error_init(ne_error_t *error) { memset(error, 0, sizeof(ne_error_t)); }

ne_error_t *ne_error_create(int error_code) {
  ne_error_t *err = malloc(sizeof(ne_error_t));
  ne_error_init(err);
  return err;
}

char *ne_error_str(ne_error_t *error){
  if (error->error_str)
    return error->error_str;
  return NULL;
}
void ne_error_free(ne_error_t *error) {
  if (error->own_str)
    free(error->error_str);
  free(error);
}
