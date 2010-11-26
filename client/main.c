#include "client.h"
#include <stdio.h>

int main (int argc, char const* argv[]) {
    t_socket connection = socket_create("eth0", 5);
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
        int e = request(&connection, buffer);

        if (e == 1) {
            response = receive(&connection); 
            if (response.type == 'X')        
                printf("%s\n", response.data);
            // free(response.data);
        }
        else if (e == 0)
            exec_command(buffer);

        // free(buffer);
    }
    return 0;
}
