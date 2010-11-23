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

    if ((dp = opendir(params)) == NULL) {
        text_message(connection, 'X', "fail");
        return;
    }

    char *output = (char *) calloc(sizeof(char), size);

    while ((dirp = readdir(dp)) != NULL) {
        while (strlen(output) + strlen(dirp->d_name) + 2 > size)
            output = realloc(output, size *= 2);

        sprintf(output, "%s%s\n", output, dirp->d_name);
//        strcpy(output + strlen(output), dirp->d_name);
 //       output = strcat(output, "\n");
    }

    closedir(dp);

    text_message(connection, 'X', output);
    free(output);
}
