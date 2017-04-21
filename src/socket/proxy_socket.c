#include "proxy_socket.h"
#include "config.h"
#include "ne_assert.h"
#include "ne_mem.h"

#define GET_PSOCK ne_proxy_socket_t *psock = socket->context

static void __socket_close_cb(ne_tcp_socket_t *socket) {
  GET_PSOCK;

  NEASSERT(psock->on_close);

  if (psock->read_buf) {
    ne_memory_buf_free(psock->read_buf);
    psock->read_buf = NULL;
  }

  if (psock->write_buf) {
    ne_memory_buf_free(psock->write_buf);
    psock->write_buf = NULL;
  }

  psock->on_close(psock);
}

static void __socket_shutdown_cb(ne_tcp_socket_t *socket) {
  GET_PSOCK;

  if (psock->write_buf) {
    ne_memory_buf_free(psock->write_buf);
    psock->write_buf = NULL;
  }

  if (psock->on_shutdown) {
    psock->on_shutdown(psock);
  }
}

static void __socket_read_cb(ne_tcp_socket_t *socket, ssize_t nread,
                             const ne_buf_t *UNUSED(buf)) {
  GET_PSOCK;

  if (nread == 0) {
    return;
  }

  if (nread > 0) {
    psock->read_pos += nread;
    psock->on_raw_read(psock);
    return;
  }

  switch (nread) {
  case NE_TCP_EOF:
    if (psock->on_EOF)
      psock->on_EOF(psock);
  case NE_GERR:
    if (psock->read_buf) {
      ne_memory_buf_free(psock->read_buf);
      psock->read_buf = NULL;
    }
    return;
  }
}

static void __socket_error_cb(ne_tcp_socket_t *socket, ne_err error) {
  GET_PSOCK;

  NEASSERT(psock->on_error);
  psock->on_error(psock, error);

  /* Since the underlying socket is closing, no need to close psock here. */
}

static void __socket_write_cb(ne_tcp_socket_t *socket, uint8_t *UNUSED(data),
                              ne_err error) {
  if (error != NE_NOERR) {
    return;
  }

  GET_PSOCK;
  NEASSERT(psock->on_raw_write);
  psock->on_raw_write(psock);
}

static void __socket_alloc_cb(ne_tcp_socket_t *socket, ne_buf_t *buf) {
  GET_PSOCK;

  if (!psock->read_buf) {
    psock->read_buf = ne_memory_pool_get_buf(psock->pool);
    NEASSERT(psock->read_buf);

    psock->reserve_size = 0;
    if (psock->reserve_read && psock->reserve_cb) {
      psock->reserve_size = psock->reserve_cb(psock);
    }
    psock->read_pos = 0;
  }

  buf->base = (char *)psock->read_buf->data + psock->read_pos;
  buf->len = psock->read_buf->size - psock->read_pos - psock->reserve_size;
}

void ne_proxy_socket_init(ne_proxy_socket_t *psock, ne_tcp_socket_t *socket,
                          ne_memory_pool_t *read_buf_pool,
                          ne_session_t *session) {
  NEMEMSET(psock, 1);
  psock->socket = socket;
  socket->context = psock;
  psock->pool = read_buf_pool;
  psock->session = session;

  socket->on_close = __socket_close_cb;
  socket->on_error = __socket_error_cb;
  socket->on_read = __socket_read_cb;
  socket->on_shutdown = __socket_shutdown_cb;
  socket->on_write = __socket_write_cb;
  socket->alloc_cb = __socket_alloc_cb;
}

void ne_proxy_socket_start(ne_proxy_socket_t *psock) {
  ne_tcp_socket_read_start(psock->socket);
}

void ne_proxy_socket_start_forwarding(ne_proxy_socket_t *psock) {
  NEASSERT(psock->on_forward_req);

  psock->on_forward_req(psock);
}

void ne_proxy_socket_close(ne_proxy_socket_t *psock) {
  ne_tcp_socket_close(psock->socket);

  if (psock->on_close_req)
    psock->on_close_req(psock);
}

void ne_proxy_socket_shutdown(ne_proxy_socket_t *psock) {
  ne_tcp_socket_shutdown(psock->socket);

  if (psock->on_shutdown_req)
    psock->on_shutdown_req(psock);
}
