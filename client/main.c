#include "client.h"
#include <stdio.h>

int main (int argc, char const* argv[]) {
    t_socket server_socket = socket_create("eth0", 10);
    char buffer[1024];
    while (1) {
        printf(" > ");
        scanf("%s", buffer);
        text_message(&server_socket, 'L', buffer);
        t_message oi = receive(&server_socket);
        printf("'%s'\n", oi.data);
    }
    return 0;
}
