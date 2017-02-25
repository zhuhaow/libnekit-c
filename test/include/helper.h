#ifndef HELPER_H
#define HELPER_H

#include "check.h"
#include "mem.h"
#include "uv.h"
#include "error.h"
#include <assert.h>
#include <string.h>

#define TEST_PORT 9123

#define TEST_MAIN                                                              \
  int main(int argc, char *argv[]) {                                           \
    int number_failed;                                                         \
    Suite *suite = build_suite();                                              \
    SRunner *runner = srunner_create(suite);                                   \
    srunner_run_all(runner, CK_NORMAL);                                        \
    number_failed = srunner_ntests_failed(runner);                             \
    srunner_free(runner);                                                      \
    return number_failed;                                                      \
  }

typedef struct {
  int socket_accept;
  int bytes_read;
  int bytes_write;
  int read_count;
  int write_count;
  int alloc_count;
  ne_error_code last_error;
} socket_stat;

typedef struct {
  uv_tcp_t handle;
  socket_stat listen_stat;
  socket_stat socket_stat;
} server_t;

void server_write_cb(uv_write_t *req, int status);

void start_server(server_t *server, uv_connection_cb conn_cb) {
  memset(server, 0, sizeof(server_t));
  assert(uv_tcp_init(uv_default_loop(), &server->handle) == 0);
  server->handle.data = server;
  struct sockaddr_in addr;
  assert(uv_ip4_addr("0.0.0.0", TEST_PORT, &addr) == 0);
  assert(uv_tcp_bind(&server->handle, (struct sockaddr *)&addr, 0) == 0);
  assert(uv_listen((uv_stream_t *)&server->handle, 30, conn_cb) == 0);
}

void vanilla_alloc_cb(uv_handle_t *handle, size_t suggested_size,
                     uv_buf_t *buf) {
  server_t *server = (server_t *)handle->data;
  server->socket_stat.alloc_count++;
  buf->base = malloc(suggested_size);
  buf->len = suggested_size;
}

void close_free_cb(uv_handle_t *stream) {
  free(stream);
}

void free_buf(const uv_buf_t *buf) {
  if (buf) {
    if (buf->base)
      free(buf->base);
  }
}

void server_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  server_t *server = (server_t *)stream->data;
  if (nread < 0) {
    free_buf(buf);
    server->socket_stat.last_error = nread;
    uv_close((uv_handle_t *)stream, close_free_cb);
    return;
  }

  server->socket_stat.read_count++;
  server->socket_stat.bytes_read += nread;

  if (nread == 0) {
    free_buf(buf);
    return;
  }

  assert(uv_read_stop(stream) == 0);
  uv_write_t *req = ALLOC(uv_write_t, 1);
  assert(req);
  uv_buf_t *write_buf = ALLOC(uv_buf_t, 1);
  write_buf->base = buf->base;
  write_buf->len = buf->len;
  req->data = write_buf;
  uv_write(req, stream, write_buf, 1, server_write_cb);
}

void server_write_cb(uv_write_t *req, int status) {
  server_t *server = (server_t *)req->handle->data;

  server->socket_stat.write_count++;

  assert(status == 0);

  uv_buf_t *write_buf = (uv_buf_t *)req->data;
  server->socket_stat.bytes_write += write_buf->len;
  free(write_buf->base);
  free(write_buf);
  free(req);

  uv_stream_t *server_sock = req->handle;
  uv_read_start(server_sock, vanilla_alloc_cb, server_read_cb);
}

void echo_server_conn_cb(uv_stream_t *server, int status) {
  assert(status == 0);

  uv_tcp_t *server_sock = ALLOC(uv_tcp_t, 1);
  assert(server_sock);
  assert(uv_tcp_init(uv_default_loop(), server_sock) == 0);
  server_sock->data = server->data;

  assert(uv_accept(server, (uv_stream_t *)server_sock) == 0);
  ((server_t *)server->data)->listen_stat.socket_accept++;
  uv_read_start((uv_stream_t *)server_sock, vanilla_alloc_cb, server_read_cb);
}

#endif /* HELPER_H */