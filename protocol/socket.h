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

#define MESSAGE_MAX_SIZE 257

int socket_create(char* device);
#endif
