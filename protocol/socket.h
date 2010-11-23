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
#include "../queue/queue.h"

#define MESSAGE_MAX_SIZE 257

typedef struct {
    int socket;
    t_queue *queue;
    unsigned window_index:8;
    unsigned window_size:8;
    unsigned sequence:8;
    unsigned index:8;
    t_message recv[256];
} t_socket;

t_socket socket_create(const char* device, const unsigned char window_size);
char *generate_packet(t_message message);
unsigned char get_parity(const char *data, int size);
t_message receive(t_socket *connection);
void handle_confirmation(t_socket *connection, const t_message packet);
void send_n_messages(t_socket *connection, unsigned char initial);
void send_message(t_socket *connection, const t_message message);
void send_raw_data(t_socket *connection, const void *data, const unsigned int size);
int recv_data(t_socket *connection, char *buffer);
void move_window(t_socket *connection);
void send_ack(t_socket *connection, const unsigned char number);
void send_nack(t_socket *connection, const unsigned char number);
void enqueue_message(t_socket *connection, t_message message);
void text_message(t_socket *connection, const char type, const char *message);

#endif
