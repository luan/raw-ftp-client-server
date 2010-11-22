#include "client.h"

void request_ls(t_socket *connection, const char *params) {
    text_message(connection, 'L', params);
}
