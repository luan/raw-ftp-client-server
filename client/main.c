#include "client.h"
#include <stdio.h>

int main (int argc, char const* argv[]) {
    t_socket server_socket = socket_create("lo");
    char buffer[1024];
    scanf("%s", buffer);
    send_message(&server_socket, 'L', buffer);
    t_message oi = receive(&server_socket);
    oi = receive(&server_socket);
    printf("'%s'\n", oi.data);
    return 0;
}
