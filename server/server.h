#ifndef _SERVER_
    #define _SERVER_

#include "../protocol/socket.h"

void respond_to(t_socket *connection, t_message packet);
void respond_ls(t_socket *connection, t_message packet);
void respond_cd(t_socket *connection, t_message packet);
void respond_put(t_socket *connection, t_message packet);
void respond_get(t_socket *connection, t_message packet);

#endif
