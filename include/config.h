#ifndef NE_CONFIG_H
#define NE_CONFIG_H

#define LISTEN_BACKLOG 30

#define SOCKET_TIMEOUT 300

#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif

#endif /* NE_CONFIG_H */
