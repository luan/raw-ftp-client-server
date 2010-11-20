#include "socket.h"

t_socket socket_create(char* device) {
    int rawconnection, deviceid;

    struct ifreq ifr;
    struct sockaddr_ll sll;
    struct packet_mreq mr;

    rawconnection = socket(PF_PACKET, SOCK_RAW, 0);
    if (rawconnection == -1)
        exit(9);

    /* get device ID */
    memset(&ifr, 0, sizeof(struct ifreq));
    memcpy(ifr.ifr_name, device, strlen(device));

    if (ioctl(rawconnection, SIOCGIFINDEX, &ifr) == -1)
        exit(1);

    deviceid = ifr.ifr_ifindex;

    memset(&sll, 0, sizeof(sll));
    sll.sll_family   = AF_PACKET;
    sll.sll_ifindex  = deviceid;
    sll.sll_protocol = htons(ETH_P_ALL);
    if (bind(rawconnection, (struct sockaddr *)&sll, sizeof(sll)) == -1)
        exit(2);

    /* Promisc mode */
    memset(&mr, 0, sizeof(mr));

    mr.mr_ifindex = deviceid;
    mr.mr_type    = PACKET_MR_PROMISC;

    if (setsockopt(rawconnection, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
        exit(3);
    
    t_socket s;
    s.socket = rawconnection;
    s.sequence = 0;
    return s;
}

char *generate_packet(t_message *message) {
    char *data = (char *) malloc(2 + message->size);
    memcpy(data, message, 4);
    memcpy(data + 4, message->data, message->size - 3);
    data[message->size + 1] = message->parity;

    return data;
}

void get_packet(const char *data, t_message *message) {
    memcpy(message, data, 4);
    message->data = (char *) malloc(sizeof(char) * message->size - 3);
    memcpy(message->data, data + 4, message->size - 3);
    message->parity = data[message->size + 1];
}

t_message receive(t_socket *connection) {
    t_message packet;
    char raw_packet[257];
    int n = recv(connection->socket, raw_packet, 257, 0);
    
    if (n < 2 || ((unsigned char) *raw_packet) != 126) {
        packet.type = 'E';
        *packet.data = (unsigned char) 5;

        return packet;
    }

    get_packet(raw_packet, &packet);

    if (get_parity(packet.data, packet.size - 3) != packet.parity)
        ;
    return packet;
}

unsigned char get_parity(const char *data, int size) {
    int i;
    unsigned char result = 0;

    for (i = 0; i < size; i++) {
        result ^= (unsigned char) data[i];
    }

    return result;
}

void send_message(t_socket *connection, const char type, const char *message) {
    int size = strlen(message);
    t_message packet;
    packet.begin = 126;
    packet.size = size + 4;
    packet.sequence = connection->sequence++;
    packet.type = type;
    packet.data = (char *) malloc(sizeof(char) * strlen(message) + 1);
    strcpy(packet.data, message);
    packet.data[size] = '\0';
    packet.parity = get_parity(packet.data, packet.size - 3);
    char *raw_packet = generate_packet(&packet);

    send_raw_data(connection, raw_packet, 2 + packet.size);
    free(raw_packet);
}

void send_raw_data(t_socket *connection, const void *data, int size) {
    send(connection->socket, data, size, 0);
}

int recv_data(t_socket *connection, char *buffer) {
    return recv(connection->socket, buffer, MESSAGE_MAX_SIZE, 0);
}
