#ifndef HELPER_H
#define HELPER_H

#include "check.h"
#include "mem.h"
#include "uv.h"
#include <assert.h>

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

void server_write_cb(uv_write_t *req, int status);

void start_server(uv_tcp_t *server, uv_connection_cb conn_cb) {
  assert(uv_tcp_init(uv_default_loop(), server) == 0);
  struct sockaddr_in addr;
  assert(uv_ip4_addr("0.0.0.0", TEST_PORT, &addr) == 0);
  assert(uv_tcp_bind(server, (struct sockaddr *)&addr, 0) == 0);
  assert(uv_listen((uv_stream_t *)server, 30, conn_cb) == 0);
}

void vanilla_alloc_cb(uv_handle_t *handle, size_t suggested_size,
                     uv_buf_t *buf) {
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
  if (nread < 0) {
    free_buf(buf);
    printf("Closed socket due to error %ld", nread);
    uv_close((uv_handle_t *)stream, close_free_cb);
    return;
  }

  if (nread == 0) {
    free_buf(buf);
    return;
  }

  assert(uv_read_stop(stream) == 0);
  uv_write_t *req = ALLOC(uv_write_t, 1);
  assert(req);
  req->data = buf->base;
  uv_write(req, stream, buf, 1, server_write_cb);
}

void server_write_cb(uv_write_t *req, int status) {
  assert(status == 0);
  uv_stream_t *server_sock = req->handle;
  free(req->data);
  free(req);
  uv_read_start(server_sock, vanilla_alloc_cb, server_read_cb);
}

void echo_server_conn_cb(uv_stream_t *server, int status) {
  assert(status == 0);

  uv_tcp_t *server_sock = ALLOC(uv_tcp_t, 1);
  assert(server_sock);
  assert(uv_tcp_init(uv_default_loop(), server_sock) == 0);

  assert(uv_accept(server, (uv_stream_t *)server_sock) == 0);
  uv_read_start((uv_stream_t *)server_sock, vanilla_alloc_cb, server_read_cb);
}

#endif /* HELPER_H */
