#include "config.h"
#include "greatest.h"
#include "ne_assert.h"
#include "ne_mem.h"
#include "proxy_socket_socks5.h"

#define PROXY_PORT 8080

ne_tcp_socket_t listen_socket;
ne_tcp_socket_t connect_socket;
ne_proxy_socket_socks5_t socks5_socket;
ne_memory_pool_t pool;
ne_session_t session;

ne_err socks5_err;
int socks5_read;
int socks5_write;
int socks5_shutdown;
int socks5_close;
int socks5_session;

struct sockaddr_in addr;

ne_tcp_socket_t *alloc_tcp_socket(ne_tcp_socket_t *UNUSED(socket)) {
  return &connect_socket;
}

void socks5_on_close(ne_proxy_socket_t *UNUSED(psock)) { socks5_close++; }

void socks5_on_read(ne_proxy_socket_socks5_t *UNUSED(psock)) { socks5_read++; }

void socks5_on_write(ne_proxy_socket_socks5_t *UNUSED(psock)) {
  socks5_write++;
}

void socks5_on_shutdown(ne_proxy_socket_socks5_t *UNUSED(psock)) {
  socks5_shutdown++;
}

void socks5_on_error(ne_proxy_socket_socks5_t *UNUSED(psock), ne_err error) {
  socks5_err = error;
}

void socks5_on_session(ne_proxy_socket_socks5_t *UNUSED(psock)) {
  socks5_session++;
}

void open_socks5(ne_tcp_socket_t *UNUSED(listen_socket),
                 ne_tcp_socket_t *socket) {
  ne_proxy_socket_socks5_init(&socks5_socket, socket, &pool, &session);
  socks5_socket.on_close = (ne_proxy_socket_event_cb)socks5_on_close;
  socks5_socket.on_write = (ne_proxy_socket_event_cb)socks5_on_write;
  socks5_socket.on_shutdown = (ne_proxy_socket_event_cb)socks5_on_shutdown;
  socks5_socket.on_close = (ne_proxy_socket_event_cb)socks5_on_close;
  ne_proxy_socket_start((ne_proxy_socket_t *)&socks5_socket);
}

void set_up_env() {
  NEASSERTE(ne_memory_pool_init(&pool, 8096, 10) == NE_NOERR);

  ne_loop_t *loop = uv_default_loop();
  ne_tcp_socket_init(loop, &listen_socket);
  listen_socket.connection_alloc = alloc_tcp_socket;
  listen_socket.on_connection = open_socks5;

  uv_ip4_addr("127.0.0.1", PROXY_PORT, &addr);
  ne_tcp_socket_bind(&listen_socket, (struct sockaddr *)&addr);
  ne_tcp_socket_listen(&listen_socket, 10);

  socks5_err = 0;
  socks5_read = 0;
  socks5_write = 0;
  socks5_shutdown = 0;
  socks5_close = 0;
  socks5_session = 0;
}

void teardown_env() {
  NEASSERTE(!uv_loop_close(uv_default_loop()));
  NEASSERTE(ne_memory_pool_free(&pool));
}

void socks5_connect1(ne_tcp_socket_t *UNUSED(s)) {
  ne_memory_buf_t *buf = ne_memory_pool_get_buf(&pool);

  buf->data[0] = 4;
  socks5_socket.write_buf = buf;
  socks5_socket.write_pos = 1;
}

void socks5_close1(ne_tcp_socket_t *UNUSED(s)) {
  ne_proxy_socket_close((ne_proxy_socket_t *)&socks5_socket);
}

TEST protocol_test1() {
  ne_tcp_socket_init(uv_default_loop(), &connect_socket);
  connect_socket.on_connect = socks5_connect1;
  ASSERT_EQ(ne_tcp_socket_connect(&connect_socket, (struct sockaddr *)&addr),
            NE_NOERR);

  ASSERT_EQ(uv_run(uv_default_loop(), UV_RUN_DEFAULT), 0);
  ASSERT(socks5_err == NE_SOCKS5_EVERSION);
  PASS();
}

SUITE(suite) {
  SET_SETUP(set_up_env, NULL);
  SET_TEARDOWN(teardown_env, NULL);

  RUN_TEST(protocol_test1);
}

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(suite);
  GREATEST_MAIN_END();
}
