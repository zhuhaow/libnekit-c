#include "check.h"
#include "helper.h"
#include "tcp_socket.h"
#include "error.h"
#include "mem.h"

#include "uv.h"

void on_connect1(ne_tcp_socket_t *socket) {
  // the corresponding server socket will close.
  ne_tcp_socket_close(socket);
  uv_close(socket->context, NULL);
}

START_TEST (tcp_socket_test) {
  uv_tcp_t server;
  start_server(&server, echo_server_conn_cb);

  ne_tcp_socket_t socket;
  ne_tcp_socket_init(uv_default_loop(), &socket);

  socket.context = &server;
  socket.on_connect = on_connect1;

  struct sockaddr_in addr;
  assert(uv_ip4_addr("127.0.0.1", TEST_PORT, &addr) == 0);
  ne_tcp_socket_connect(&socket, (struct sockaddr *)&addr);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  ck_assert(uv_loop_close(uv_default_loop()) == 0);
}
END_TEST

Suite *build_suite() {
  Suite *s;
  TCase *tcp_case;

  s = suite_create("TCP Socket");

  tcp_case = tcase_create("Socket");
  tcase_add_test(tcp_case, tcp_socket_test);

  suite_add_tcase(s, tcp_case);

  return s;
}

TEST_MAIN
