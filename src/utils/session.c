#include "session.h"
#include "ne_mem.h"

void ne_session_init(ne_session_t *session) { NEMEMSET(session, 1); }

void ne_session_free(ne_session_t *session) {
  if (session->host)
    free(session->host);

  for (size_t i = 0; i < session->in_len; ++i) {
    free(session->in_addrs + i);
  }

  for (size_t i = 0; i < session->in6_len; ++i) {
    free(session->in6_addrs + i);
  }
}
