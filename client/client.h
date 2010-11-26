#ifndef _CLIENT_
    #define _CLIENT_

#include "../protocol/socket.h"
void exec_command(const char * command);
void exec_ls(const char *params);
void exec_cd(const char *params);
int request(t_socket *connection, const char *command);
void request_put(t_socket *connection, const char *params);
void request_get(t_socket *connection, const char *params);
void request_cd(t_socket *connection, const char *params);
void request_ls(t_socket *connection, const char *params);

#endif

