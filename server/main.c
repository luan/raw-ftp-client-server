#include "server.h"

int main (int argc, char const* argv[]) {
    int server_socket = socket_create("lo");
    printf("server on\n");
    char buffer[1024];
    recv(server_socket, buffer, 1023, 0);
    printf("'%s'\n", buffer);
    return 0;
}
