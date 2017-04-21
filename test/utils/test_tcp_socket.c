#include "error.h"
#include "greatest.h"
#include "helper.h"
#include "tcp_socket.h"

#include "uv.h"

#include "config.h"
#include "ne_assert.h"

#include <stdio.h>

bool connected;
uv_timer_t timer;

uint8_t hello_data[] = "hello";

void set_up_env() { connected = false; }

/* TEST CASE 1:
   Connect than disconnect.
 */

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

TEST connect_test1() {
  uv_timer_init(uv_default_loop(), &timer);

  server_t server;
  start_server(&server, echo_server_conn_cb);

  ne_tcp_socket_t socket;
  ne_tcp_socket_init(uv_default_loop(), &socket);

  socket.context = &server;
  socket.on_connect = on_connect1;

  struct sockaddr_in addr;
  NEASSERTE(uv_ip4_addr("127.0.0.1", TEST_PORT, &addr) == 0);
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

/* TEST CASE 1 END */

/* TEST CASE 2:
   Connect, send "hello" then disconnect without read. Should send back RST.

   I personally believe there is a bug in OS X (and maybe iOS) that the socket
   would
   send FIN then RST, which is not correct, but reported here:
   https://patchwork.kernel.org/patch/9210023/

   Anyway, this behavior should not matter that much but we should definitely
   keey that in mind.
 */

void timer_cb2(uv_timer_t *handle) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)handle->data;
  ne_tcp_socket_close(socket);
  uv_close((uv_handle_t *)&((server_t *)socket->context)->handle, NULL);
  uv_close((uv_handle_t *)handle, NULL);
}

void on_connect2(ne_tcp_socket_t *socket) {
  connected = true;
  ne_tcp_socket_write(socket, hello_data, 6);
}

void on_write2(ne_tcp_socket_t *socket, uint8_t *data, ne_err UNUSED(err)) {
  NEASSERT(data == hello_data);
  timer.data = socket;
  uv_timer_start(&timer, timer_cb1, 50, 0);
}

TEST connect_test2() {
  uv_timer_init(uv_default_loop(), &timer);

  server_t server;
  start_server(&server, echo_server_conn_cb);

  ne_tcp_socket_t socket;
  ne_tcp_socket_init(uv_default_loop(), &socket);

  socket.context = &server;
  socket.on_connect = on_connect2;
  socket.on_write = on_write2;

  struct sockaddr_in addr;
  NEASSERTE(uv_ip4_addr("127.0.0.1", TEST_PORT, &addr) == 0);
  ne_tcp_socket_connect(&socket, (struct sockaddr *)&addr);

  ASSERT_EQ(uv_run(uv_default_loop(), UV_RUN_DEFAULT), 0);

  ASSERT_EQ(uv_loop_close(uv_default_loop()), 0);

  ASSERT(connected);

  ASSERT_EQ(server.listen_stat.socket_accept, 1);
  ASSERT_EQ(server.listen_stat.last_error, 0);

#ifndef __APPLE__
  ASSERT_EQ(server.socket_stat.last_error, UV_ECONNRESET);
#endif
  ASSERT(server.socket_stat.alloc_count == 1 ||
         server.socket_stat.alloc_count == 2);
  ASSERT_EQ(server.socket_stat.read_count, 1);
  ASSERT_EQ(server.socket_stat.bytes_read, 6);
  ASSERT_EQ(server.socket_stat.write_count, 1);
  ASSERT_EQ(server.socket_stat.bytes_write, 6);

  PASS();
}

/* TEST CASE 2 END */

/* TEST CASE 3:
   Try to read and write back "hello" for a few rounds.
 */

int current_round = 1;
int max_round = 10;

void timer_cb3(uv_timer_t *handle) {
  ne_tcp_socket_t *socket = (ne_tcp_socket_t *)handle->data;
  ne_tcp_socket_close(socket);
  uv_close((uv_handle_t *)&((server_t *)socket->context)->handle, NULL);
  uv_close((uv_handle_t *)handle, NULL);
}

void on_connect3(ne_tcp_socket_t *socket) {
  connected = true;
  ne_tcp_socket_write(socket, hello_data, 6);
  ne_tcp_socket_read_start(socket);
}

void on_write3(ne_tcp_socket_t *UNUSED(socket), uint8_t *data, ne_err UNUSED(err)) {
  NEASSERT(data == hello_data);
}

void on_read3(ne_tcp_socket_t *socket, ssize_t nread, const ne_buf_t *buf) {
  NEASSERT(nread == 6);
  NEASSERTE(!memcmp(buf->base, hello_data, 6));
  free(buf->base);
  if (current_round < max_round) {
    current_round++;
    ne_tcp_socket_write(socket, hello_data, 6);
  } else {
    timer.data = socket;
    uv_timer_start(&timer, timer_cb1, 50, 0);
  }
}

void alloc_cb3(ne_tcp_socket_t *UNUSED(socket), ne_buf_t *buf) {
  buf->base = malloc(30);
  buf->len = 30;
}

TEST connect_test3() {
  uv_timer_init(uv_default_loop(), &timer);

  server_t server;
  start_server(&server, echo_server_conn_cb);

  ne_tcp_socket_t socket;
  ne_tcp_socket_init(uv_default_loop(), &socket);

  socket.context = &server;
  socket.on_connect = on_connect3;
  socket.on_write = on_write3;
  socket.on_read = on_read3;
  socket.alloc_cb = alloc_cb3;

  struct sockaddr_in addr;
  NEASSERTE(uv_ip4_addr("127.0.0.1", TEST_PORT, &addr) == 0);
  ne_tcp_socket_connect(&socket, (struct sockaddr *)&addr);

  ASSERT_EQ(uv_run(uv_default_loop(), UV_RUN_DEFAULT), 0);

  ASSERT_EQ(uv_loop_close(uv_default_loop()), 0);

  ASSERT(connected);

  ASSERT_EQ(server.listen_stat.socket_accept, 1);
  ASSERT_EQ(server.listen_stat.last_error, 0);

  ASSERT_EQ(server.socket_stat.last_error, UV_EOF);
  ASSERT(server.socket_stat.alloc_count == 10 ||
         server.socket_stat.alloc_count == 11);
  ASSERT_EQ(server.socket_stat.read_count, 10);
  ASSERT_EQ(server.socket_stat.bytes_read, 60);
  ASSERT_EQ(server.socket_stat.write_count, 10);
  ASSERT_EQ(server.socket_stat.bytes_write, 60);

  PASS();
}

/* TEST CASE 3 END */

/* TEST CASE 4: Try to bind the same address and port to multiple tcp socket. */
TEST bind_test1() {
  ne_tcp_socket_t socket1, socket2;
  ne_tcp_socket_init(uv_default_loop(), &socket1);
  ne_tcp_socket_init(uv_default_loop(), &socket2);

  struct sockaddr_in addr;
  ASSERT(uv_ip4_addr("127.0.0.1", TEST_PORT, &addr) == 0);
  ne_tcp_socket_bind(&socket1, (struct sockaddr *)&addr);
  ne_tcp_socket_bind(&socket2, (struct sockaddr *)&addr);

  ne_tcp_socket_close(&socket1);
  ne_tcp_socket_close(&socket2);

  PASS();
}

/* TEST CASE 4 END */

SUITE(suite) {
  SET_SETUP(set_up_env, NULL);

  RUN_TEST(connect_test1);
  RUN_TEST(connect_test2);
  RUN_TEST(connect_test3);
  RUN_TEST(bind_test1);
}

GREATEST_MAIN_DEFS();

TEST_MAIN
