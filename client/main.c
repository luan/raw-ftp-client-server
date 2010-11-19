#include "client.h"

int main (int argc, char const* argv[]) {
    int server_socket = socket_create("lo");
    char buffer[1024];
    scanf("%s", buffer);
    send(server_socket, buffer, 1023, 0);
    return 0;
}
