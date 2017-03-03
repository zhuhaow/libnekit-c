#include "greatest.h"
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
  uv_close((uv_handle_t *)&((server_t *)socket->context)->handle, NULL);
  uv_close((uv_handle_t *)handle, NULL);
}

void on_connect1(ne_tcp_socket_t *socket) {
  // the corresponding server socket will close.
  connected = true;
  timer.data = socket;
  uv_timer_start(&timer, timer_cb1, 50, 0);
}

TEST connect_test() {
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

  ASSERT_EQ(uv_run(uv_default_loop(), UV_RUN_DEFAULT), 0);

  ASSERT_EQ(uv_loop_close(uv_default_loop()), 0);

  ASSERT(connected);

  ASSERT_EQ(server.listen_stat.socket_accept, 1);
  ASSERT_EQ(server.listen_stat.last_error, 0);

  ASSERT_EQ(server.socket_stat.last_error, UV_EOF);
  ASSERT_IN_RANGE(server.socket_stat.alloc_count, 0, 1);
  ASSERT_EQ(server.socket_stat.bytes_read, 0);
  ASSERT_EQ(server.socket_stat.read_count, 0);
  ASSERT_EQ(server.socket_stat.bytes_write, 0);
  ASSERT_EQ(server.socket_stat.write_count, 0);

  PASS();
}

SUITE(suite) {
  RUN_TEST(connect_test);
}

GREATEST_MAIN_DEFS();

TEST_MAIN
