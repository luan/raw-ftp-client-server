#ifndef _SOCKET_
#define _SOCKET_

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "protocol.h"

#define MESSAGE_MAX_SIZE 257

typedef struct {
    int socket;
    unsigned sequence:8;
} t_socket;

t_socket socket_create(char* device);
char *generate_packet(t_message *message);
unsigned char get_parity(const char *data, int size);
t_message receive(t_socket *connection);
void send_message(t_socket *connection, const char type, const char *message);
void send_raw_data(t_socket *connection, const void *data, int size);
int recv_data(t_socket *connection, char *buffer);
#endif
