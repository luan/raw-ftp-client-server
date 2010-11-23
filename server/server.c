#include "server.h"

void respond_to(t_socket *connection, t_message packet) {
    if (packet.type == 'E' && ((unsigned short) *packet.data) == 5)
        return;

    if (packet.type == TYPE_LS)
        respond_ls(connection, packet);
    else if (packet.type == TYPE_CD)
        respond_cd(connection, packet);
}

void respond_cd(t_socket *connection, t_message packet) {
    char *params = (char *) malloc(sizeof(char) * packet.size - 2);
    strncpy(params, packet.data, packet.size - 3);
    params[packet.size - 3] = '\0';

    char buffer[2048];
    
    if (!cd(params)) {
        sprintf(buffer, "remote chdir at '%s'\n", params);
        text_message(connection, 'X', buffer);
    }
    else if (errno == EACCES)
        enqueue_message(connection, error_message(2));
    else
        enqueue_message(connection, error_message(1));
}

void respond_ls(t_socket *connection, t_message packet) {
    char *params = (char *) malloc(sizeof(char) * packet.size - 2);
    strncpy(params, packet.data, packet.size - 3);
    params[packet.size - 3] = '\0';

    char *output = (char *) calloc(1048576, 1);
    ls(params, output);
    text_message(connection, 'X', output);
    free(output);
}
