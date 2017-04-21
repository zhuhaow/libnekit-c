#ifndef NE_PROXY_SOCKET_SOCKS5_H
#define NE_PROXY_SOCKET_SOCKS5_H

#include "proxy_socket.h"

typedef enum { PS5RVERSION = 0, PS5RREQUEST, PS5RFORWARDING } ne_proxy_socket_socks5_read_status;

typedef enum { PS5WVERSION = 0, PS5WRESPONSE, PS5WFORWARDING } ne_proxy_socket_socks5_write_status;

typedef struct {
  PROXY_SOCKET_FIELDS
  uint8_t addr_type;
  ne_proxy_socket_socks5_read_status read_status;
  ne_proxy_socket_socks5_write_status write_status;
} ne_proxy_socket_socks5_t;

void ne_proxy_socket_socks5_init(ne_proxy_socket_socks5_t *psock,
                                 ne_tcp_socket_t *socket,
                                 ne_memory_pool_t *pool,
                                 ne_session_t *session);
#endif /* NE_PROXY_SOCKET_SOCKS5_H */
