#include "log.h"
#include "ne_assert.h"
#include <stdio.h>

int log_level;

void ne_log_set_level(int level) {
  NEASSERT(level >= NELOG_DEBUG && level <= NELOG_ERROR);
  log_level = level;
}

int ne_log_get_level() { return log_level; }
