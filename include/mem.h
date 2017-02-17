#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include <string.h>

#define ALLOC(t,n) (t *) calloc(sizeof(t), n)

#define MEMSET(p,n) memset(p, 0, sizeof(*p) * n);

#endif /* MEM_H */
