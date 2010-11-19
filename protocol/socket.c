#include "socket.h"

int socket_create(char* device) {
    int raw_socket, deviceid;

    struct ifreq ifr;
    struct sockaddr_ll sll;
    struct packet_mreq mr;

    raw_socket = socket(PF_PACKET, SOCK_RAW, 0);
    if (raw_socket == -1)
        exit(9);

    /* get device ID */
    memset(&ifr, 0, sizeof(struct ifreq));
    memcpy(ifr.ifr_name, device, strlen(device));

    if (ioctl(raw_socket, SIOCGIFINDEX, &ifr) == -1)
        exit(1);

    deviceid = ifr.ifr_ifindex;

    memset(&sll, 0, sizeof(sll));
    sll.sll_family   = AF_PACKET;
    sll.sll_ifindex  = deviceid;
    sll.sll_protocol = htons(ETH_P_ALL);
    if (bind(raw_socket, (struct sockaddr *)&sll, sizeof(sll)) == -1)
        exit(2);

    /* Promisc mode */
    memset(&mr, 0, sizeof(mr));

    mr.mr_ifindex = deviceid;
    mr.mr_type    = PACKET_MR_PROMISC;

    if (setsockopt(raw_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
        exit(3);

    return raw_socket;
}

void generate_packet(t_message *message, char *data) {
    data = (char *) malloc(2 + message->size);
    memcpy(data, message, 4);
    memcpy(data + 4, message->data, message->size - 3);
    memcpy(data + 1 + message->size, message->parity, 1);
}

void send_message(int _socket, const char *message) {
    int size = strlen(message);

    send_raw_data(_socket, message, size);
}

void send_raw_data(int _socket, const char *data, int size) {
    send(_socket, data, size, 0);
}

int recv_data(int _socket, char *buffer) {
    return recv(_socket, buffer, MESSAGE_MAX_SIZE, 0);
}
