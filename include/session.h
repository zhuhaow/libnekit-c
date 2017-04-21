#ifndef NE_SESSION_H
#define NE_SESSION_H

#include <uv.h>

typedef struct {
  char *host;
  uint16_t port;
  struct sockaddr_in *in_addrs;
  size_t in_len;
  struct sockaddr_in6 *in6_addrs;
  size_t in6_len;
} ne_session_t;

void ne_session_init(ne_session_t *session);
void ne_session_free(ne_session_t *session);

#endif /* NE_SESSION_H */
