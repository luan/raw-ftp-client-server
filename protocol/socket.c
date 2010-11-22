#include "socket.h"

t_socket socket_create(const char* device, const unsigned char window_size) {
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
    s.window_index = 0;
    s.window_size = window_size;
    s.queue = queue_new();
    s.sequence = 0;

    int i;

    for (i = 0; i < 256; i++) {
        s.messages_confirmation[i] = 0;
    }
    return s;
}

char *generate_packet(t_message message) {
    char *data = (char *) malloc(2 + message.size);
    memcpy(data, &message, 4);
    memcpy(data + 4, message.data, message.size - 3);
    data[message.size + 1] = message.parity;

    return data;
}

void get_packet(const char *data, t_message *message) {
    memcpy(message, data, 4);
    message->data = (char *) malloc(sizeof(char) * message->size - 3);
    memcpy(message->data, data + 4, message->size - 3);
    message->parity = data[message->size + 1];
}

int timeoutable_recv(t_socket *connection, char *data, unsigned timeout_secs) {
    struct timeval timeout;
    fd_set rfds;

    timeout.tv_sec = timeout_secs;
    timeout.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(connection->socket, &rfds);

    if (select(connection->socket + 1, &rfds, NULL, NULL, &timeout))
        return recv_data(connection, data);
    else
        return -1;
}

t_message create_message(const void *data, const unsigned char size, const char type) {
    t_message message;
    message.begin = 126;
    message.size = size + 2;
    message.type = type;
    message.data = (void *) malloc(size);
    memcpy(message.data, data, size);
    message.parity = get_parity(data, size);
    return message;
}

void enqueue_splited(t_socket *connection, const char *data, const unsigned size, const char type) {
    unsigned i;
    
    for (i = 0; i < size; i += 252) {
        enqueue_message(connection, create_message(data + i, ((size - i) > 252) ? 252 : (size - i), type));
    }
}

t_message error_message(unsigned char errno) {
    t_message message;
    message.type = 'E';
    *message.data = errno;

    return message;
}

void perform_confirmation(t_socket *connection) {
    int i, j, ack = 1;

    for (i = 0; i < connection->window_size; i++) {
        unsigned char index = connection->window_index + i;
        if (!connection->messages_confirmation[index]) {
            for (j = i + 1; j < connection->window_size; j++) {
                unsigned char index_j = connection->window_index + j;
                if (connection->messages_confirmation[index_j])
                    ack = 0;
            }

            break;
        }
    }

    if (ack)
        send_ack(connection, connection->window_index + i - 1);
    else
        send_nack(connection, connection->window_index + i);
}

t_message receive(t_socket *connection) {
    t_message packet;
    char raw_packet[257];

    int n = timeoutable_recv(connection, raw_packet, 5);

    if (n < 0) {
        send_n_messages(connection, connection->queue->value.sequence);
        return receive(connection);
    }
    else if (n < 5 || ((unsigned char) *raw_packet) != 126)
        return error_message(5);

    get_packet(raw_packet, &packet);

    if (packet.sequence >= connection->window_index && packet.sequence < (connection->window_index + connection->window_size))
        return error_message(6);

    if (get_parity(packet.data, packet.size - 3) != packet.parity) {
        send_nack(connection, packet.sequence);
        return receive(connection);
    }

    if (connection->messages_confirmation[packet.sequence] == 1)
        return error_message(7);

    connection->sequence++;
    connection->messages_confirmation[packet.sequence] = 1;

    perform_confirmation(connection);

    if (packet.type == 'Y' || packet.type == 'N')
        handle_confirmation(connection, packet);

    return packet;
}

void handle_confirmation(t_socket *connection, const t_message packet) {
    unsigned char next = connection->window_index + connection->window_size;
    unsigned char aux = 0;
    if (packet.type == 'N')
        aux = -1;

    t_message m = dequeue_until(&connection->queue, packet.sequence + aux);
    unsigned char before = connection->window_index;
    connection->window_index = packet.sequence + 1 + aux;

    unsigned char i;

    for (i = 0; i < connection->window_index - before; i++) {
        unsigned char index = before + connection->window_size + i;
        connection->messages_confirmation[index] = 0;
    }

    if (packet.type == 'N')
        send_message(connection, m);

    send_n_messages(connection, next);
}

void send_n_messages(t_socket *connection, unsigned char initial) {
    t_queue *q;
    for (q = connection->queue; q->value.sequence != initial && q->next; q = q->next);
    for (; q && q->value.sequence < (connection->window_index + connection->window_size); q = q->next)
        send_message(connection, q->value);
}

unsigned char get_parity(const char *data, int size) {
    int i;
    unsigned char result = 0;

    for (i = 0; i < size; i++) {
        result ^= (unsigned char) data[i];
    }

    return result;
}

void enqueue_message(t_socket *connection, t_message message) {
    message.sequence = connection->sequence++;
    connection->messages_confirmation[message.sequence] = 1;
    enqueue(connection->queue, message);

    if (queue_size(connection->queue) <= connection->window_size)
        send_message(connection, message);
}

void text_message(t_socket *connection, const char type, const char *message) {
    int size = strlen(message);
    if (size > 252) {
        enqueue_message(connection, create_message(&size, 0, 'A'));
        enqueue_splited(connection, message, size, type);
        enqueue_message(connection, create_message("", 0, 'B'));
    }
    else
        enqueue_splited(connection, message, size, type);
}

void send_confirmation(t_socket *connection, const unsigned char number, const char type) {
    t_message packet = create_message("", 0, type);
    packet.sequence = number;

    char *raw_packet = generate_packet(packet);

    send_raw_data(connection, raw_packet, 5);
    free(raw_packet);
}

void send_ack(t_socket *connection, const unsigned char number) {
    send_confirmation(connection, number, 'Y');
}

void send_nack(t_socket *connection, const unsigned char number) {
    send_confirmation(connection, number, 'N');
}

void send_message(t_socket *connection, const t_message message) {
    char *raw_packet = generate_packet(message);
    send_raw_data(connection, raw_packet, message.size + 2);
    free(raw_packet);
}

void send_raw_data(t_socket *connection, const void *data, const unsigned int size) {
    send(connection->socket, data, size, 0);
}

int recv_data(t_socket *connection, char *buffer) {
    return recv(connection->socket, buffer, MESSAGE_MAX_SIZE, 0);
}
