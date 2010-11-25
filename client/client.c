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

    char *initial_cmd = strtrim(&cmd);
    char *initial_params = strtrim(&params);
    int retval = 1;

    if (!strcmp(cmd, "ls"))
        request_ls(connection, params);
    else if (!strcmp(cmd, "cd"))
        request_cd(connection, params);
    else if (!strcmp(cmd, "put")) {
        request_put(connection, params);
        retval = 2;
    }
    else
        retval =  0;

    free(initial_cmd);
    free(initial_params);
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
    
    char *initial_cmd = strtrim(&cmd);
    char *initial_params = strtrim(&params);

    if (!strcmp(cmd, "lls"))
        exec_ls(params);
    else if (!strcmp(cmd, "lcd"))
        exec_cd(params);
    else
        printf("invalid command\n");

    free(initial_params);
    free(initial_cmd);
}

void exec_ls(const char *params) {
    char *output = (char *) calloc(1048576, 1);
    ls(params, output);
    printf("%s\n", output);
    if (output != NULL)
        free(output);
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
        if (c.type == 'N')
            handle_confirmation(connection, c);
    } while (c.type != 'Y' || c.type == 'E');

    if (c.type == 'E') {
        switch ((unsigned char) *c.data) {
            case 2:
                printf("permission denied\n");
                break;
            case 3:
                printf("not enough space\n");
                break;
            case 4:
                printf("file not found\n");
        }
        c.type = 'Y';
        handle_confirmation(connection, c);
        return;
    }

    handle_confirmation(connection, c);
    // create file
    c = receive(connection);
    // file size
    // receive file
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
        if (c.type == 'N')
            handle_confirmation(connection, c);
    } while (c.type != 'Y' || c.type == 'E');

    if (c.type == 'E') {
        if ((unsigned char) *c.data == 2)
            printf("permission denied\n");

        c.type = 'Y';
        handle_confirmation(connection, c);
        return;
    }

    handle_confirmation(connection, c);
    send_file(connection, params);
}

void request_ls(t_socket *connection, const char *params) {
    text_message(connection, TYPE_LS, params);
}

void request_cd(t_socket *connection, const char *params) {
    text_message(connection, TYPE_CD, params);
}
