#ifndef NE_TYPES_H_
#define NE_TYPES_H_

#include <stddef.h>
#include <uv.h>

#define bool uint8_t
#define true 1
#define false 0

#define ne_buf_t uv_buf_t
#define ne_loop_t uv_loop_t

#define ne_timer_t uv_timer_t

#endif
