#include "client.h"

void request_ls(t_socket *connection, const char *params) {
    send_message(connection, 'L', params);
}
