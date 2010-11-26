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
#include <sys/time.h>
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
#include "../lib/sysutil/sysutil.h"

#define MESSAGE_MAX_SIZE 257
#define MAX_FILE_QUEUE 1024

typedef struct {
    int socket;
    t_queue *queue;
    unsigned window_index:8;
    unsigned window_size;
    unsigned sequence:8;
    unsigned index:8;
    unsigned count:5;
    unsigned sign:1;
    t_message recv[256];
} t_socket;

t_socket socket_create(const char* device, const unsigned window_size);
char *generate_packet(t_message message);
unsigned char get_parity(const char *data, int size);
void handle_confirmation(t_socket *connection, t_message message);
t_message recv_message(t_socket *connection);
t_message receive(t_socket *connection);
t_message error_message(unsigned char err, unsigned char sequence);
void print_progress(unsigned long total, unsigned long size, unsigned starttime, int reverse);
int send_file(t_socket *connection, const char *filename, int progress_bar);
void send_message(t_socket *connection, const t_message message);
void send_raw_data(t_socket *connection, const void *data, const unsigned int size);
int recv_data(t_socket *connection, char *buffer);
void move_window(t_socket *connection);
void send_ack(t_socket *connection, const unsigned char number);
void send_nack(t_socket *connection, const unsigned char number);
void enqueue_message(t_socket *connection, t_message message);
void text_message(t_socket *connection, const char type, const char *message);

#endif
