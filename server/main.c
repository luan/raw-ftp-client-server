#include "server.h"

int main (int argc, char const* argv[]) {
    t_socket server_socket = socket_create("lo", 10);
    printf("server on\n");
    t_message packet;

    while (1) {
        packet = receive(&server_socket);
        respond_to(&server_socket, packet);
    }
    return 0;
}
