#ifndef _SERVER_
    #define _SERVER_

#include "../protocol/socket.h"
#include "../lib/sysutil/sysutil.h"

void respond_to(t_socket *connection, t_message packet);
void respond_ls(t_socket *connection, t_message packet);
void respond_cd(t_socket *connection, t_message packet);
#endif
