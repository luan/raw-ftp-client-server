#include "server.h"

void respond_to(t_socket *connection, t_message packet) {
    if (packet.type == TYPE_ERR && ((unsigned short) *packet.data) == 5)
        return;

    if (packet.type == TYPE_LS)
        respond_ls(connection, packet);
    else if (packet.type == TYPE_CD)
        respond_cd(connection, packet);
    else if (packet.type == TYPE_PUT)
        respond_put(connection, packet);
    else if (packet.type == TYPE_GET)
        respond_get(connection, packet);
}

void respond_cd(t_socket *connection, t_message packet) {
    char *params = (char *) malloc(sizeof(char) * packet.size - 2);
    strncpy(params, packet.data, packet.size - 3);
    params[packet.size - 3] = '\0';

    char buffer[2048];
    
    if (!cd(params)) {
        sprintf(buffer, "remote chdir at '%s'\n", params);
        text_message(connection, TYPE_SCREEN, buffer);
    }
    else if (errno == EACCES)
        enqueue_message(connection, error_message(2, packet.sequence));
    else
        enqueue_message(connection, error_message(1, packet.sequence));

    if (params != NULL)
        free(params);
}

void respond_ls(t_socket *connection, t_message packet) {
    char *params = (char *) malloc(sizeof(char) * packet.size - 2);
    strncpy(params, packet.data, packet.size - 3);
    params[packet.size - 3] = '\0';

    char *output = (char *) calloc(1048576, 1);
    ls(params, output);
    text_message(connection, TYPE_SCREEN, output);
    if (output != NULL)
        free(output);
    if (params != NULL)
        free(params);
}

void respond_put(t_socket *connection, t_message packet) {
    t_message message = receive(connection);
    unsigned long size;
    memcpy(&size, message.data, sizeof(unsigned long));

    if (size > free_disk_space()) {
        send_message(connection, error_message(3, message.sequence));
        return;
    }
    
    send_ack(connection, message.sequence);

    FILE *fp = fopen(packet.data, "wb");

    t_message next;

    do {
        next = receive(connection);
        if (next.type == TYPE_EOF)
            break;
        fwrite(next.data, next.size - 3, 1, fp);
        //free(next.data);
    } while (1);

    fclose(fp);

    message = next;
}

void respond_get(t_socket *connection, t_message packet) {
    char *params = (char *) malloc(sizeof(char) * packet.size - 2);
    strncpy(params, packet.data, packet.size - 3);
    params[packet.size - 3] = '\0';

    if (!file_exists(params)) {
        send_message(connection, error_message(4, packet.size));
        return;
    }

    send_ack(connection, packet.sequence);
    send_file(connection, params, 0);
    free(params);
}
