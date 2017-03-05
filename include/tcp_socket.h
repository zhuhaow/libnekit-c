#ifndef NE_TCP_SOCKET_H
#define NE_TCP_SOCKET_H

#include <uv.h>

#include "types.h"

typedef ssize_t ne_tcp_socket_err;
#define NE_TCP_NOERR 0
#define NE_TCP_ERST -1
#define NE_TCP_TIMEOUT -2

typedef ne_tcp_socket_err ne_tcp_socket_read_err;
#define NE_TCP_RNOERR 0
#define NE_TCP_REOF -1
#define NE_TCP_RERR -2

typedef ne_tcp_socket_err ne_tcp_socket_write_err;
#define NE_TCP_WNOERR 0
#define NE_TCP_WERR -1

typedef ne_tcp_socket_err ne_tcp_socket_connect_err;
#define NE_TCP_CNOERR 0
#define NE_TCP_CEADDRINUSE -1

struct ne_tcp_socket;
typedef struct ne_tcp_socket ne_tcp_socket_t;

typedef void (*ne_tcp_socket_cb)(ne_tcp_socket_t *socket);
typedef void (*ne_tcp_socket_status_cb)(ne_tcp_socket_t *socket,
                                        ne_tcp_socket_err status);
typedef void (*ne_tcp_socket_alloc_cb)(ne_tcp_socket_t *socket, ne_buf_t *buf);
typedef void (*ne_tcp_socket_read_cb)(ne_tcp_socket_t *socket, ssize_t nread,
                                      const ne_buf_t *buf);

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

typedef union {
  uv_handle_t handle;
  uv_stream_t stream;
  uv_tcp_t tcp;
} ne_uv_tcp_handle_t;

/* The TCP socket. */
struct ne_tcp_socket {
  /* The underlying uv_handle. */
  ne_uv_tcp_handle_t handle;

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
  /* The callback when the socket read data encountered some
   * `ne_tcp_socket_read_err`. One should check if `nread` is `NE_TCP_RNOERR`.
   * If not, then the read is failed and stopped (no need to call
   * `ne_tcp_socket_read_stop`) and any buffer allocated for reading should be
   * released now. The socket should never be read again.
   */
  ne_tcp_socket_read_cb on_read;
  /* The callback to set up the ne_buf_t buffer for reading. */
  ne_tcp_socket_alloc_cb alloc_cb;

  /* The callback when the queued write request is finished or encounters some
   * error. One should check the `status`. If it is other then `NE_TCP_WNOERR`,
   * then the write is failed and the buffer should be relaesed. The socket
   * should never be wrote again. */
  ne_tcp_socket_status_cb on_write;
  ne_buf_t write_buf;

  ne_tcp_socket_cb on_close;
  ne_tcp_socket_cb on_shutdown;
  /* An unrecoverable error happened and the socket will be closed
   * automatically. */
  ne_tcp_socket_status_cb on_error;

  /* Only set it (in milliseconds) after `ne_tcp_socket_init` if you don't like
   * the default. Set it to 0 to disable time out. */
  uint64_t timeout;

  /* Private */
  uv_connect_t connect_req;
  uv_write_t write_req;
  uv_shutdown_t shutdown_req;
  ne_timer_t timeout_timer;

  bool timer_closed;
  bool socket_closed;
};

/* Initialize a new tcp socket on event loop. */
void ne_tcp_socket_init(ne_loop_t *loop, ne_tcp_socket_t *socket);

/* Set up the socket if the socket is accepted from a listening socket,
   instead of connecting actively. */
void ne_tcp_socket_accepted(ne_tcp_socket_t *socket);

/* Bind the socket to some address. */
void ne_tcp_socket_bind(ne_tcp_socket_t *socket, struct sockaddr *addr);

/* Connect to remote address. */
ne_tcp_socket_connect_err ne_tcp_socket_connect(ne_tcp_socket_t *socket, struct sockaddr *addr);

/* Start reading data, `on_read` will be called if there is data available. */
int ne_tcp_socket_read_start(ne_tcp_socket_t *socket);

/* Stop reading data. */
void ne_tcp_socket_read_stop(ne_tcp_socket_t *socket);

/* Send data according to `write_req`. */
void ne_tcp_socket_write(ne_tcp_socket_t *socket);

/* Shutdown the socket (sending FIN). */
void ne_tcp_socket_shutdown(ne_tcp_socket_t *socket);

/* Close the socket, must be called before it is released. */
void ne_tcp_socket_close(ne_tcp_socket_t *socket);

#endif /* NE_TCP_SOCKET_H */
