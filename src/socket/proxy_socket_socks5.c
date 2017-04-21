#include "proxy_socket_socks5.h"
#include "config.h"
#include "ne_assert.h"
#include "ne_mem.h"

#define GET_PSOCK ne_proxy_socket_socks5_t *psock = socket->context
#define TO_PSOCK(ssock) (ne_proxy_socket_t *)ssock

static inline void __notify_error(ne_proxy_socket_socks5_t *psock, ne_err err) {
  if (psock->on_error) {
    psock->on_error(TO_PSOCK(psock), err);
  }

  ne_proxy_socket_close(TO_PSOCK(psock));
}

static inline void __write_socket(ne_proxy_socket_socks5_t *psock) {
  NEASSERT(psock->write_buf);
  NEASSERT(psock->write_pos);

  ne_tcp_socket_write(psock->socket, psock->write_buf->data, psock->write_pos);
}

static void __socket_read_handle(ne_proxy_socket_socks5_t *psock) {
  uint8_t *data = psock->read_buf->data;
  size_t data_len = psock->read_pos;

  switch (psock->read_status) {
  case PS5RFORWARDING:
    ne_tcp_socket_read_stop(psock->socket);
    psock->on_read(TO_PSOCK(psock));
    break;
  case PS5RVERSION:
    if (*data != 5) {
      __notify_error(psock, NE_SOCKS5_EVERSION);
      return;
    }

    if (data_len < 2) {
      return;
    }

    if (*(data + 1) + 2 < data_len) {
      return;
    }

    if (*(data + 1) == 0 || *(data + 1) + 2 > data_len) {
      __notify_error(psock, NE_SOCKS5_EAUTHMETHOD);
      return;
    }

    bool afound = false;
    for (size_t i = 0; i < *(data + 1); ++i) {
      if (*(data + i + 2) == 0) {
        afound = true;
        break;
      }
    }

    if (!afound) {
      __notify_error(psock, NE_SOCKS5_EAUTHMETHOD);
      return;
    };

    psock->write_buf = psock->read_buf;
    psock->read_buf = NULL;

    ne_tcp_socket_read_stop(psock->socket);

    *psock->write_buf->data = 5;
    *(psock->write_buf->data + 1) = 0;
    psock->write_pos = 2;
    __write_socket(psock);

    break;
  case PS5RREQUEST:
    /* VER | CMD | RSV | ATYP | DST.ADDR | DST.PORT */
    /* The length is clearly at least 8 */
    if (data_len < 8) {
      return;
    }

    if (*data != 5) {
      __notify_error(psock, NE_SOCKS5_EVERSION);
      return;
    }

    if (*(data + 1) != 1) {
      __notify_error(psock, NE_SOCKS5_ECOMMAND);
      return;
    }

    NEASSERT(psock->on_session);
    psock->addr_type = *(data + 3);
    switch (psock->addr_type) {
    case 1: {
      if (data_len < 10) {
        return;
      }

      if (data_len > 10) {
        __notify_error(psock, NE_SOCKS5_EIPV4ADDR);
        return;
      }

      psock->session->in_addrs = malloc(sizeof(struct sockaddr_in));
      if (!psock->session->in_addrs) {
        __notify_error(psock, NE_ENOMEM);
        return;
      }

      psock->session->in_len = 1;
      struct sockaddr_in *addr = psock->session->in_addrs;
      addr->sin_family = AF_INET;
      memcpy(&addr->sin_addr, data + 4, 4);
      memcpy(&addr->sin_port, data + 8, 2);

      ne_memory_buf_free(psock->read_buf);
      ne_tcp_socket_read_stop(psock->socket);

      psock->on_session(TO_PSOCK(psock));
    } break;
    case 3: {
      if (*(data + 4) + 7 > data_len) {
        return;
      }

      if (*(data + 4) + 7 < data_len) {
        __notify_error(psock, NE_SOCKS5_EDEST);
        return;
      }

      memcpy(psock->session->host, data + 5, *(data + 4));
      psock->session->port = ntohs(*(uint16_t *)(data + 4 + *(data + 4)));

      ne_memory_buf_free(psock->read_buf);
      ne_tcp_socket_read_stop(psock->socket);

      psock->on_session(TO_PSOCK(psock));
    } break;
    case 4: {
      if (data_len < 24) {
        return;
      }

      if (data_len > 24) {
        __notify_error(psock, NE_SOCKS5_EIPV4ADDR);
        return;
      }

      psock->session->in6_addrs = malloc(sizeof(struct sockaddr_in6));
      if (!psock->session->in6_addrs) {
        __notify_error(psock, NE_ENOMEM);
        return;
      }

      psock->session->in6_len = 1;
      struct sockaddr_in6 *addr = psock->session->in6_addrs;
      addr->sin6_family = AF_INET6;
      memcpy(&addr->sin6_addr, data + 4, 16);
      memcpy(&addr->sin6_port, data + 20, 2);

      ne_memory_buf_free(psock->read_buf);
      ne_tcp_socket_read_stop(psock->socket);

      psock->on_session(TO_PSOCK(psock));
    } break;
    default:
      __notify_error(psock, NE_SOCKS5_EUNKNOWNADDRTYPE);
      ne_proxy_socket_close(TO_PSOCK(psock));
      return;
    }
  }

  return;
}

static void __socket_write_handle(ne_proxy_socket_socks5_t *psock) {
  switch (psock->write_status) {
  case PS5WFORWARDING:
    ne_memory_buf_free(psock->write_buf);
    ne_tcp_socket_read_start(psock->socket);
    return;
  case PS5WVERSION:
    ne_memory_buf_free(psock->write_buf);
    psock->read_status = PS5RREQUEST;
    ne_tcp_socket_read_start(psock->socket);
    return;
  case PS5WRESPONSE:
    ne_memory_buf_free(psock->write_buf);
    psock->read_status = PS5RFORWARDING;
    psock->write_status = PS5WFORWARDING;
    ne_tcp_socket_read_start(psock->socket);
    return;
  }
}

static void __socket_forward_handle(ne_proxy_socket_socks5_t *psock) {
  NEASSERT(psock->write_status = PS5WVERSION);
  NEASSERT(psock->read_status = PS5RREQUEST);

  psock->write_status = PS5WRESPONSE;
  psock->write_buf = ne_memory_pool_get_buf(psock->pool);

  if (!psock->write_buf) {
    __notify_error(psock, NE_ENOMEM);
    return;
  }

  uint8_t *data = psock->write_buf->data;

  *data = 5;
  *(data + 1) = 0;
  *(data + 2) = 0;
  if (psock->addr_type == 3)
    *(data + 3) = 1;
  else
    *(data + 3) = psock->addr_type;

  if (psock->addr_type == 4) {
    memset(data + 4, 0, 18);
    psock->write_pos = 22;
  } else {
    memset(data + 4, 0, 6);
    psock->write_pos = 10;
  }

  __write_socket(psock);
}

void ne_proxy_socket_socks5_init(ne_proxy_socket_socks5_t *psock,
                                 ne_tcp_socket_t *socket,
                                 ne_memory_pool_t *pool,
                                 ne_session_t *session) {
  NEMEMSET(psock, 1);
  ne_proxy_socket_init(TO_PSOCK(psock), socket, pool, session);
  psock->type = NE_SOCKS5_PROXY;

  psock->on_raw_read = (ne_proxy_socket_event_cb)__socket_read_handle;
  psock->on_raw_write = (ne_proxy_socket_event_cb)__socket_write_handle;
  psock->on_forward_req = (ne_proxy_socket_event_cb)__socket_forward_handle;
}

