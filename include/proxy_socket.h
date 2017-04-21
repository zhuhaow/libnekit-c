#ifndef NE_PROXY_SOCKET_H
#define NE_PROXY_SOCKET_H

#include "memory_pool.h"
#include "session.h"
#include "tcp_socket.h"

enum ne_proxy_type { NE_HTTP_PROXY, NE_SOCKS5_PROXY };

typedef struct ne_proxy_socket_s ne_proxy_socket_t;

typedef size_t (*ne_proxy_socket_read_reserve_cb)(ne_proxy_socket_t *);
typedef void (*ne_proxy_socket_event_cb)(ne_proxy_socket_t *);
typedef void (*ne_proxy_socket_status_cb)(ne_proxy_socket_t *, ne_err);

#define PROXY_SOCKET_FIELDS                                                    \
  void *context;                                                               \
  enum ne_proxy_type type;                                                     \
  ne_session_t *session;                                                       \
  ne_tcp_socket_t *socket;                                                     \
  ne_memory_pool_t *pool;                                                      \
  ne_memory_buf_t *read_buf;                                                   \
  bool reserve_read;                                                           \
  size_t reserve_size;                                                         \
  size_t read_pos;                                                             \
  ne_memory_buf_t *write_buf;                                                  \
  size_t write_pos;                                                            \
  ne_err error;                                                                \
  ne_proxy_socket_read_reserve_cb reserve_cb;                                  \
  ne_proxy_socket_event_cb on_close, on_read, on_write, on_shutdown,           \
      on_session, on_EOF;                                                      \
  ne_proxy_socket_event_cb on_raw_read, on_raw_write, on_close_req,            \
      on_shutdown_req, on_forward_req;                                             \
  ne_proxy_socket_status_cb on_error;

struct ne_proxy_socket_s {
  PROXY_SOCKET_FIELDS
};

void ne_proxy_socket_init(ne_proxy_socket_t *psock, ne_tcp_socket_t *socket,
                          ne_memory_pool_t *read_buf_pool,
                          ne_session_t *session);
void ne_proxy_socket_start(ne_proxy_socket_t *psock);
void ne_proxy_socket_start_forwarding(ne_proxy_socket_t *psock);
void ne_proxy_socket_shutdown(ne_proxy_socket_t *psock);
void ne_proxy_socket_close(ne_proxy_socket_t *psock);

#endif /* NE_PROXY_SOCKET_H */
