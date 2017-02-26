#include "check.h"
#include "helper.h"
#include "tcp_socket.h"
#include "error.h"
#include "mem.h"

#include "uv.h"

#include <stdio.h>


bool connected;
uv_timer_t timer;

void timer_cb1(uv_timer_t *handle) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)handle->data;
  ne_tcp_socket_close(socket);
  uv_close(socket->context, NULL);
  uv_close(handle, NULL);
}

void on_connect1(ne_tcp_socket_t *socket) {
  // the corresponding server socket will close.
  connected = true;
  timer.data = socket;
  uv_timer_start(&timer, timer_cb1, 0, 0);
}

START_TEST (connect_test) {
  uv_timer_init(uv_default_loop(), &timer);

  server_t server;
  start_server(&server, echo_server_conn_cb);

  ne_tcp_socket_t socket;
  ne_tcp_socket_init(uv_default_loop(), &socket);
  connected = false;

  socket.context = &server;
  socket.on_connect = on_connect1;

  struct sockaddr_in addr;
  assert(uv_ip4_addr("127.0.0.1", TEST_PORT, &addr) == 0);
  ne_tcp_socket_connect(&socket, (struct sockaddr *)&addr);

  ck_assert_int_eq(uv_run(uv_default_loop(), UV_RUN_DEFAULT), 0);

  ck_assert_int_eq(uv_loop_close(uv_default_loop()), 0);

  ck_assert(connected);

  ck_assert_int_eq(server.listen_stat.socket_accept, 1);
  ck_assert_int_eq(server.listen_stat.last_error, 0);

  ck_assert_int_eq(server.socket_stat.last_error, UV_EOF);
  ck_assert_int_le(server.socket_stat.alloc_count, 1);
  ck_assert_int_eq(server.socket_stat.bytes_read, 0);
  ck_assert_int_eq(server.socket_stat.read_count, 0);
  ck_assert_int_eq(server.socket_stat.bytes_write, 0);
  ck_assert_int_eq(server.socket_stat.write_count, 0);
}
END_TEST

Suite *build_suite() {
  Suite *s;
  TCase *tcp_case;

  s = suite_create("TCP Socket");

  tcp_case = tcase_create("Socket connect");
  tcase_add_test(tcp_case, connect_test);

  suite_add_tcase(s, tcp_case);

  return s;
}

TEST_MAIN
