#ifndef NE_MEM_H
#define NE_MEM_H

#include <stdlib.h>
#include <string.h>

#define ALLOC(t,n) (t *) calloc(sizeof(t), n)

#define MEMSET(p,n) memset(p, 0, sizeof(*p) * n);

#endif /* NE_MEM_H */
