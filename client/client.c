#include "client.h"

int request(t_socket *connection, const char *command) {
    char *cmd = (char *) calloc(16, sizeof(char));
    char *params = (char *) calloc(256, sizeof(char));

    int i = 0, j = 0;
    while (command[i] != ' ' && command[i] != '\0') {
        cmd[i] = command[i];
        i++;
    }
    cmd[i] = '\0';

    if (command[i] == ' ')
        while (command[i] != '\0')
            params[j++] = command[i++];

    /*char *initial_cmd = */strtrim(&cmd);
    /*char *initial_params = */strtrim(&params);
    int retval = 1;

    if (!strcmp(cmd, "ls"))
        request_ls(connection, params);
    else if (!strcmp(cmd, "cd"))
        request_cd(connection, params);
    else if (!strcmp(cmd, "put")) {
        request_put(connection, params);
        retval = 2;
    }
    else if (!strcmp(cmd, "get")) {
        request_get(connection, params);
        retval = 2;
    }
    else
        retval =  0;

    // free(initial_cmd);
    // free(initial_params);
    return retval;
}

void exec_command(const char * command) {
    char *cmd = (char *) calloc(16, sizeof(char));
    char *params = (char *) calloc(256, sizeof(char));

    int i = 0, j = 0;
    while (command[i] != ' ' && command[i] != '\0') {
        cmd[i] = command[i];
        i++;
    }
    cmd[i] = '\0';

    if (command[i] == ' ')
        while (command[i] != '\0')
            params[j++] = command[i++];
    
    /*char *initial_cmd = */strtrim(&cmd);
    /*char *initial_params = */strtrim(&params);

    if (!strcmp(cmd, "lls"))
        exec_ls(params);
    else if (!strcmp(cmd, "lcd"))
        exec_cd(params);
    else
        printf("invalid command\n");

    // free(initial_params);
    // free(initial_cmd);
}

void exec_ls(const char *params) {
    char *output = (char *) calloc(1048576, 1);
    ls(params, output);
    printf("%s\n", output);
    // free(output);
}

void exec_cd(const char *params) {
    if (!cd(params))
        printf("locally chdir at '%s'\n", params);
    else if (errno == EACCES)
        printf("access denied hacker biatch(\n");
    else
        printf("no hay la pastita que tu quieres\n");
}

void request_get(t_socket *connection, const char *params) {
    text_message(connection, TYPE_GET, params);
    t_message c;
    do {
        c = recv_message(connection);
        if (c.type == TYPE_NACK)
            handle_confirmation(connection, c);
    } while (c.type != TYPE_ACK && c.type != TYPE_ERR);

    if (c.type == TYPE_ERR) {
        switch ((unsigned char) *c.data) {
            case 4:
                printf("file not found\n");
        }
        c.type = TYPE_ACK;
        handle_confirmation(connection, c);
        return;
    }

    handle_confirmation(connection, c);

    c = receive(connection);

    if (c.type != TYPE_FILE) {
        printf("file not found\n");
        return;
    }        

    unsigned int size;
    memcpy(&size, c.data, sizeof(unsigned long));
    
    if (size > free_disk_space()) {
        printf("NO HAY ESPACITO EN LO DISCO AMIGO\n");
        send_message(connection, error_message(3, c.sequence));
        connection->window_index = c.sequence + 1;
        return;
    }

    send_ack(connection, c.sequence);

    FILE *fp = fopen(params, "wb");
    struct timeval sttime;
    gettimeofday(&sttime, NULL);
    unsigned starttime = sttime.tv_sec;

    t_message next;

    do {
        next = receive(connection);
        if (next.type == TYPE_EOF)
            break;
        if (next.type == TYPE_DATA) {
            print_progress(size, size - ftell(fp), starttime, 1);
            fwrite(next.data, next.size - 3, 1, fp);
            free(next.data);
        }
        else {
            printf("transfer error\n");
            return;
        }
    } while (1);

    print_progress(size, size - ftell(fp), starttime, 1);
    printf("\n");

    fclose(fp);
    free(next.data);
}

void request_put(t_socket *connection, const char *params) {
    if (!file_exists(params)) {
        printf("file not found\n");
        return;
    }

    text_message(connection, TYPE_PUT, params);
    t_message c;
    do {
        c = recv_message(connection);
        if (c.type == TYPE_NACK)
            handle_confirmation(connection, c);
    } while (c.type != TYPE_ACK && c.type != TYPE_ERR);

    if (c.type == TYPE_ERR) {
        if ((unsigned char) *c.data == 2)
            printf("permission denied\n");

        c.type = TYPE_ACK;
        handle_confirmation(connection, c);
        return;
    }

    handle_confirmation(connection, c);
    send_file(connection, params, 1);
}

void request_ls(t_socket *connection, const char *params) {
    text_message(connection, TYPE_LS, params);
}

void request_cd(t_socket *connection, const char *params) {
    text_message(connection, TYPE_CD, params);
}
