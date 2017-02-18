#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "tcp_socket.h"
#include "config.h"

void ne_tcp_socket_init(ne_loop_t *loop, ne_tcp_socket_t *socket) {
  memset(socket, 0, sizeof(ne_tcp_socket_t));

  socket->status = INVALID;

  uv_tcp_init(loop, &socket->handle);
  socket->handle.data = socket;

  socket->connect_req.data = socket;
  socket->write_req.data = socket;
  socket->shutdown_req.data = socket;

  uv_timer_init(loop, &socket->timeout_timer);
  socket->timeout_timer.data = socket;
  socket->timeout = SOCKET_TIMEOUT;
}

void __tcp_socket_timeout_cb(uv_timer_t *handle) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)handle->data;
  if (socket->on_error)
    socket->on_error(socket, UV_ETIMEDOUT);
  ne_tcp_socket_close(socket);
}

void __tcp_socket_timer_reset(ne_tcp_socket_t *socket) {
  uv_timer_again(&socket->timeout_timer);
}

void __tcp_socket_timer_start(ne_tcp_socket_t *socket) {
  if (socket->timeout > 0)
    uv_timer_start(&socket->timeout_timer, __tcp_socket_timeout_cb,
                   socket->timeout, socket->timeout);
}

void __tcp_socket_timer_stop(ne_tcp_socket_t *socket) {
  uv_timer_stop(&socket->timeout_timer);
}


void __tcp_socket_check_close(ne_tcp_socket_t *socket) {
  if (socket->timer_closed && socket->socket_closed) {
    socket->status = CLOSED;
    if (socket->on_close)
      socket->on_close(socket);
  }
}

void __tcp_socket_socket_close_cb(uv_handle_t *handle) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)handle->data;
  socket->socket_closed = true;
  __tcp_socket_check_close(socket);
}

void __tcp_socket_timer_close_cb(uv_handle_t *handle) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)handle->data;
  socket->timer_closed = true;
  __tcp_socket_check_close(socket);
}

void ne_tcp_socket_close(ne_tcp_socket_t *socket) {
  socket->status = CLOSING;
  uv_close((uv_handle_t *)&socket->timeout_timer, __tcp_socket_timer_close_cb);
  uv_close((uv_handle_t *)&socket->handle, __tcp_socket_socket_close_cb);
}

/* Return true when the error is handled and the caller should just return.
   Return false when there is no error or the caller should handle it. */
bool __tcp_socket_handle_error(ne_tcp_socket_t *socket, int status) {
  if (status > 0)
    return false;

  if (socket->status >= CLOSING)
    return true;

  switch (status) {
  case UV_EOF:
    return false;
  case UV_ETIMEDOUT:
  case UV_ECONNRESET:
  case UV_ENETDOWN:
  case UV_ENETUNREACH:
    socket->on_error(socket, status);
    ne_tcp_socket_close(socket);
    return true;
  case UV_ECANCELED:
    /* This should only happen when the socket is closing, which is already checked. */
    assert(false);
  }
  return false;
}

void ne_tcp_socket_accepted(ne_tcp_socket_t *socket) {
  socket->status = CONNECTED;
  __tcp_socket_timer_start(socket);
}

int ne_tcp_socket_bind(ne_tcp_socket_t *socket, struct sockaddr *addr) {
  return uv_tcp_bind(&socket->handle, addr, 0);
}

void __tcp_socket_connect_cb(uv_connect_t *req, int status) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)req->data;

  if (__tcp_socket_handle_error(socket, status))
    return;

  assert(status == 0);

  socket->on_connect(socket);
}

int ne_tcp_socket_connect(ne_tcp_socket_t *socket, struct sockaddr *addr) {
  int r;

  r = uv_tcp_connect(&socket->connect_req, &socket->handle, addr,
                     __tcp_socket_connect_cb);
  if (!r)
    socket->status = CONNECTING;

  return r;
}

void ne_tcp_socket_read_stop(ne_tcp_socket_t *socket) {
  socket->reading = false;
  uv_read_stop((uv_stream_t *)&socket->handle);
}

void __tcp_socket_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)stream->data;

  if (__tcp_socket_handle_error(socket, nread))
    return;

  __tcp_socket_timer_reset(socket);

  if (nread == UV_EOF) {
    socket->receiveEOF = true;
    ne_tcp_socket_read_stop(socket);
  } else {
    assert(nread >= 0);
  }

  socket->on_read(socket, nread, buf);
}

void __tcp_socket_alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)handle->data;

  socket->alloc_cb(socket, buf);
}

int ne_tcp_socket_read_start(ne_tcp_socket_t *socket) {
  socket->reading = true;
  return uv_read_start((uv_stream_t *)&socket->handle, __tcp_socket_alloc_cb,
                __tcp_socket_read_cb);
}

void __tcp_socket_write_cb(uv_write_t *req, int status) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)req->data;
  if (__tcp_socket_handle_error(socket, status))
    return;

  assert(status == 0);
  socket->on_write(socket);
}

void ne_tcp_socket_write(ne_tcp_socket_t *socket) {
  uv_write(&socket->write_req, (uv_stream_t *)&socket->handle,
           &socket->write_buf, 1, __tcp_socket_write_cb);
}

void __tcp_socket_shutdown_cb(uv_shutdown_t *req, int status) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)req->data;
  if (__tcp_socket_handle_error(socket, status))
    return;

  assert(status == 0);
  socket->status = SHUT_DOWN;
  socket->on_shutdown(socket);
}
void ne_tcp_socket_shutdown(ne_tcp_socket_t *socket) {
  assert(socket->status == CONNECTING || socket->status == CONNECTED);
  socket->status = SHUTTING_DOWN;
  uv_shutdown(&socket->shutdown_req, (uv_stream_t *)&socket->handle,
              __tcp_socket_shutdown_cb);
}

void ne_tcp_socket_free(ne_tcp_socket_t *socket) {
  assert(socket->status == CLOSED);
  free(socket);
}
