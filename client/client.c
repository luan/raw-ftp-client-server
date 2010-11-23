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

    strtrim(&cmd);
    strtrim(&params);

    if (!strcmp(cmd, "ls"))
        request_ls(connection, params);
    else if (!strcmp(cmd, "cd"))
        request_cd(connection, params);
    else
        return 0;

    return 1;
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

    strtrim(&cmd);
    strtrim(&params);

    if (!strcmp(cmd, "lls"))
        exec_ls(params);
    else if (!strcmp(cmd, "lcd"))
        exec_cd(params);
    else
        printf("invalid command\n");
}

void exec_ls(const char *params) {
    char *output = (char *) calloc(1048576, 1);
    ls(params, output);
    printf("%s\n", output);
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

void request_ls(t_socket *connection, const char *params) {
    text_message(connection, TYPE_LS, params);
}

void request_cd(t_socket *connection, const char *params) {
    text_message(connection, TYPE_CD, params);
}
