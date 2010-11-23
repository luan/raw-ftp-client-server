#include "client.h"
#include <stdio.h>

int main (int argc, char const* argv[]) {
    t_socket connection = socket_create("eth0", 10);
    t_message response;

    while (1) {
        char *buffer = calloc(1024, sizeof(char));
        printf(" > ");

        int i = 0;
        char c;

        while ((c = getchar()) != '\n') {
            buffer[i++] = c;
        }

        buffer[i] = '\0';

        if (request(&connection, buffer))
            response = receive(&connection); 
        else
            exec_command(buffer);

        if (response.type == 'X')        
            printf("'%s'\n", response.data);
        free(buffer);
    }
    return 0;
}
