#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <uv.h>

#include "types.h"

struct ne_tcp_socket;
typedef struct ne_tcp_socket ne_tcp_socket_t;

typedef void (*ne_tcp_socket_cb)(ne_tcp_socket_t *socket);
typedef void (*ne_tcp_socket_status_cb)(ne_tcp_socket_t *socket, int status);
typedef void (*ne_tcp_socket_alloc_cb)(ne_tcp_socket_t *socket, ne_buf_t *buf);
typedef void (*ne_tcp_socket_read_cb)(ne_tcp_socket_t *socket, int nread, const ne_buf_t *buf);

enum ne_tcp_socket_status {
  /* The initial state of socket */
  INVALID = 0,
  /* The socket is connecting to remote. */
  CONNECTING,
  /* The socket is connected, ready to read and write. */
  CONNECTED,
  /* The socket is shutting down. */
  SHUTTING_DOWN,
  /* The socket has shut down. */
  SHUT_DOWN,
  /* The socket is closing (see uv_close in libuv) */
  CLOSING,
  /* The socket can be freed now. */
  CLOSED
};

/* The TCP socket. */
struct ne_tcp_socket {
  /* The underlying uv_handle. */
  uv_tcp_t handle;

  /* User context */
  void *context;

  /* Current status of the socket. */
  enum ne_tcp_socket_status status;

  /* The callback when the socket connects to remote successfully. */
  ne_tcp_socket_cb on_connect;

  /* The socket is reading data. */
  bool reading;
  /* The socket has received EOF, so nothing more will be read,
     and ne_tcp_socket_start_read should never be called again. */
  bool receiveEOF;
  /* The callback when the socket read data or EOF. */
  ne_tcp_socket_read_cb on_read;
  /* The callback to set up the ne_buf_t buffer for reading. */
  ne_tcp_socket_alloc_cb alloc_cb;

  /* The callback when the queued write request is finished. */
  ne_tcp_socket_cb on_write;
  ne_buf_t write_buf;

  ne_tcp_socket_cb on_close;
  ne_tcp_socket_cb on_shutdown;
  /* An unrecoverable error happened and the socket will be closed automatically. */
  ne_tcp_socket_status_cb on_error;

  int timeout;

  /* Private */
  uv_connect_t connect_req;
  uv_write_t write_req;
  uv_shutdown_t shutdown_req;
  ne_timer_t timeout_timer;

  bool timer_closed;
  bool socket_closed;
};

void ne_tcp_socket_init(ne_loop_t *loop, ne_tcp_socket_t *socket);
void ne_tcp_socket_accepted(ne_tcp_socket_t *socket);
int ne_tcp_socket_bind(ne_tcp_socket_t *socket, struct sockaddr *addr);
int ne_tcp_socket_connect(ne_tcp_socket_t *socket, struct sockaddr *addr);
int ne_tcp_socket_read_start(ne_tcp_socket_t *socket);
void ne_tcp_socket_read_stop(ne_tcp_socket_t *socket);
void ne_tcp_socket_write(ne_tcp_socket_t *socket);
void ne_tcp_socket_shutdown(ne_tcp_socket_t *socket);
void ne_tcp_socket_close(ne_tcp_socket_t *socket);
void ne_tcp_socket_free(ne_tcp_socket_t *socket);

#endif /* TCP_SOCKET_H */
