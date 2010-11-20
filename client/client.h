#ifndef _CLIENT_
    #define _CLIENT_

#include "../protocol/socket.h"

void request_ls(t_socket *connection, const char *params);

#endif

