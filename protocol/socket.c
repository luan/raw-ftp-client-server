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
    s.index = 0;

    int i;

    for (i = 0; i < 256; i++) {
        s.recv[i].begin = 0;
    }
    return s;
}

char *generate_packet(t_message message) {
    char *data = (char *) malloc(2 + message.size);
    memcpy(data, &message, 4);

    if (message.size - 2)
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

t_message create_message(const char *data, const unsigned char size, const char type) {
    t_message message;
    message.begin = 126;
    message.size = size + 3;
    message.type = type;
    message.data = (void *) malloc(size);
    memcpy(message.data, data, size);
    message.parity = get_parity(data, size);
    return message;
}

void enqueue_splited(t_socket *connection, const char *data, const unsigned size, const char type) {
    unsigned i;

    for (i = 0; i < size; i += 252) {
        unsigned index = ((size - i) > 252) ? 252 : size - i;
        t_message next = create_message(data + i, index, type);
        enqueue_message(connection, next);
    }
}

t_message error_message(unsigned char errno) {
    t_message message;
    message.type = 'E';
    message.data = malloc(1);
    *message.data = errno;

    return message;
}

t_message receive(t_socket *connection) {
    t_message message;
    char raw_packet[257];

    if (connection->recv[connection->window_index].begin) {
        t_message m = connection->recv[connection->window_index++];
        return m;
    }

    int n = timeoutable_recv(connection, raw_packet, 5);

    if (n < 0) {
        unsigned char i;
        t_queue *q = connection->queue;

        for (i = 0; q && q->value.begin && i < connection->window_size; i++, q = q->next) {
            send_message(connection, q->value);
        }

        return receive(connection);
    }
    else if (n < 5 || ((unsigned char) *raw_packet) != 126){
        return receive(connection);
    }

    get_packet(raw_packet, &message);

    if (message.type == 'Y' || message.type == 'N') {
        unsigned char sequence = (message.type == 'Y') ? message.sequence : message.sequence - 1;

        if (has_element(connection->queue, sequence)) {
            unsigned char num = 0;

            t_message message;
            do {
                message = dequeue(&connection->queue);
                num++;
            } while(sequence != message.sequence);

            unsigned char i;
            t_queue *q = connection->queue;

            for (i = 0; i < connection->window_size; i++) {
                if (!q->next || !q->value.begin)
                    return receive(connection);
                q = q->next;
            }

            for (i = 0; q && i < num && q->value.begin; i++, q = q->next) {
                send_message(connection, q->value);
            }
        }

        return receive(connection);
    }

    if (message.sequence == connection->window_index) {
        send_ack(connection, message.sequence);
        connection->window_index++;

        if (message.type == 'A') {
            char type;
            unsigned int size;
            memcpy(&size, message.data, 4);
            memcpy(&type, message.data + 4, 1);

            if (type == 'X')
                size++;

            message.type = type;
            message.data = (char *) malloc(size);

            t_message next;
            unsigned int i = 0;

            do {
                next = receive(connection);
                memcpy(message.data + i, next.data, next.size - 3);
                i += next.size - 3;
            } while (next.type != 'B');

            message.data[size - 1] = '\0';
        }

        return message;
    }
    else {
        unsigned char i;
        
        for (i = 0; i < connection->window_size; i++) {
            unsigned char index = connection->window_index + i;

            if (!connection->recv[index].begin) {
                send_nack(connection, index);
                break;
            }
        }

        connection->recv[message.sequence] = message;
        return receive(connection);
    }

    /*
    t_message packet;
    char raw_packet[257];

    printf("%d\n", check_cable(connection));
    if (!check_cable(connection) && connection->recv[connection->window_index].begin) {
        printf("cable\n");
        t_message result = connection->recv[connection->window_index];
        perform_confirmation(connection);

        if (packet.type == 'A') {
            int size;
            char type;

            memcpy(&size, packet.data, 4);
            memcpy(&type, packet.data + 4, 1);

            if (type == 'X')
                size++;

            packet.data = calloc(size, 1);
            int i = 0;
            t_message next;

            do {
                next = receive(connection);
                if (next.type != type) {
                    continue;
                }
                next.size -= 3;
                memcpy(packet.data + i, next.data, next.size);
                i += next.size;
            } while (next.type != 'B');

            packet.size = size + 3;
            packet.type = type;
        }
        return result;
    }

    int n = timeoutable_recv(connection, raw_packet, 5);

    if (n < 0) {
        send_n_messages(connection, connection->queue->value.sequence);
        return receive(connection);
    }
    else if (n < 5 || ((unsigned char) *raw_packet) != 126){
        return receive(connection);
    }

    get_packet(raw_packet, &packet);
    
    if (packet.type == 'Y' || packet.type  == 'N') {
        handle_confirmation(connection, packet);
        return receive(connection);
    }

    if (get_parity(packet.data, packet.size - 3) != packet.parity) {
        send_nack(connection, packet.sequence);
        return receive(connection);
    }
    
    connection->recv[packet.sequence] = packet;
    
    printf("recv\n");

    return receive(connection);

    // if (connection->messages_confirmation[packet.sequence])
    //    return receive(connection);printf("confirmed %d\n", packet.sequence);


    // if (packet.sequence < connection->window_index && packet.sequence >= (connection->window_index + connection->window_size))
    //    return error_message(6);*/
}

void handle_confirmation(t_socket *connection, const t_message packet) {
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
    enqueue(connection->queue, message);

    if (queue_size(connection->queue) <= connection->window_size) {
        send_message(connection, message);
    }
}

void text_message(t_socket *connection, const char type, const char *message) {
    int size = strlen(message);
    if (size > 252) {
        char *data = malloc(5);
        memcpy(data, &size, 4);
        memcpy(data + 4, &type, 1);
        enqueue_message(connection, create_message(data, sizeof(int) + 1, 'A'));
        enqueue_splited(connection, message, size, type);
        enqueue_message(connection, create_message("", 0, 'B'));
    }
    else {
        enqueue_splited(connection, message, size, type);
    }
}

void send_confirmation(t_socket *connection, const unsigned char number, const char type) {
    t_message packet = create_message("", 0, type);
    packet.sequence = number;
    printf("%c %d\n", type, number);

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
    printf("send[%03d]: %c\n", message.sequence, message.type);
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
