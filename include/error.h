#ifndef NE_ERROR_H
#define NE_ERROR_H

#include "types.h"

typedef ssize_t ne_err;

/* General error */
#define NE_NOERR 0
#define NE_GERR -1

#define NE_ENOMEM -2

/* TCP socket error */
#define NE_TCP_ERST -100
#define NE_TCP_ETIMEOUT -101
#define NE_TCP_ENETUNREACH -102
#define NE_TCP_EOF -103
#define NE_TCP_EADDRINUSE -104

/* SOCKS5 server error */
#define NE_SOCKS5_EVERSION -200
#define NE_SOCKS5_EAUTHMETHOD -201
#define NE_SOCKS5_ECOMMAND -202
#define NE_SOCKS5_EIPV4ADDR -203
#define NE_SOCKS5_EDEST -204
#define NE_SOCKS5_EUNKNOWNADDRTYPE -205


#endif /* NE_ERROR_H */
