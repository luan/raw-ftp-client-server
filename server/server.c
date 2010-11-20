#include "server.h"

void respond_to(t_socket *connection, t_message packet) {
    if (packet.type == 'E' && ((unsigned short) *packet.data) == 5)
        return;

    if (packet.type == 'L')
        respond_ls(connection, packet);
}

void respond_ls(t_socket *connection, t_message packet) {
    DIR *dp;
    struct dirent *dirp;

    //if (argc != 2) {
    //    fprintf(stderr, "usage: %s dir_name\n", argv[0]);
    //    exit(1);
    //}
    
    char *params = (char *) malloc(sizeof(char) * packet.size - 2);
    strncpy(params, packet.data, packet.size - 3);
    params[packet.size - 3] = '\0';
    int size = 16;
    char *output = (char *) malloc(sizeof(char) * size);

    if ((dp = opendir(params)) == NULL ) {
        fprintf(stderr, "can't open '%s'\n", params);
        exit(1);
    }

    while ((dirp = readdir(dp)) != NULL) {
        if ((strlen(output) + strlen(dirp->d_name) + 2) > size)
            output = realloc(output, size *= 2);
        strcpy(output + strlen(output), dirp->d_name);
        output = strcat(output, "\n");
    }

    closedir(dp);

    send_message(connection, 'X', output);
    free(output);
}
