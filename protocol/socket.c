#include "socket.h"

t_socket socket_create(const char* device, const unsigned window_size) {
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

    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

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
    message.data = (void *) calloc(size, 1);
    memcpy(message.data, data, size);
    message.parity = get_parity(data, size);
    return message;
}

void enqueue_splited(t_socket *connection, const char *data, const unsigned size, const char type) {
    unsigned i;

    for (i = 0; i <= size; i += 252) {
        unsigned index = ((size - i) > 252) ? 252 : size - i;
        t_message next = create_message(data + i, index, type);
        enqueue_message(connection, next);
    }
}

t_message error_message(unsigned char err, unsigned char sequence) {
    t_message m = create_message((char *) &err, 1, TYPE_ERR);
    m.sequence = sequence;
    return m;
}

void handle_confirmation(t_socket *connection, t_message message) {
    unsigned char sequence = message.sequence;
 
    if (message.type == TYPE_NACK) {
        t_message r = get_element(connection->queue, message.sequence);
        if (r.begin) {
            send_message(connection, r);
        }
        return;
    }

    if (has_element(connection->queue, sequence)) {
        unsigned char num = 0;

        t_message c;
        do {
            c = dequeue(&connection->queue);
            num++;
        } while(sequence != c.sequence);

        unsigned char i;
        t_queue *q = connection->queue;

        for (i = 0; i < connection->window_size; i++) {
            if (!q->next || !q->value.begin)
                return;
            q = q->next;
        }

        for (i = 0; q && i < num && q->value.begin; i++, q = q->next) {
            send_message(connection, q->value);
        }
    }
}

t_message recv_message(t_socket *connection) {
    t_message message;
    char raw_packet[257];

    if (connection->recv[connection->window_index].begin) {
        t_message m = connection->recv[connection->window_index];
        if (m.parity != get_parity(m.data, m.size - 3)) {
            connection->recv[connection->window_index].begin = 0;
        }
    }

    int n = -1;

    while (n < 0) {
        n = timeoutable_recv(connection, raw_packet, 5);

        if (n < 0) {
            unsigned char i;
            t_queue *q = connection->queue;

            for (i = 0; q->value.begin && i < connection->window_size; i++, q = q->next)
                send_message(connection, q->value);
        }
        else if (n < 5 || ((unsigned char) *raw_packet) != 126)
            n = -1;
        else {
            get_packet(raw_packet, &message);
        }
    }

    return message;
}

void recalculate_window(t_socket *connection) {
    connection->count++;
    if (connection->count % 20 == 0) {
        if (connection->sign) {
            connection->sign = 0;
            connection->window_size /= 2;
            if (!connection->window_size)
                connection->window_size = 1;
        }
        else {
            connection->sign = 1;
            connection->window_size *= 2;
        }

        connection->count = 0;
    }

}

t_message receive(t_socket *connection) {
    t_message message = recv_message(connection);

    while (message.parity != get_parity(message.data, message.size - 3)) {
        message = recv_message(connection);
    }

    if (message.type == TYPE_ERR) {
        switch ((unsigned char) *message.data) {
            case 1:
                printf("you don't have teh folder\n");
                break;
            case 2:
                printf("BEHH! YOU CANNOT ACCESS HERE MORON!\n");
                break;
            default:
                send_ack(connection, message.sequence);
        }
    }

    if (message.type == TYPE_ACK || message.type == TYPE_NACK) {
        handle_confirmation(connection, message);
        free(message.data);

        return receive(connection);
    }

    recalculate_window(connection);

    if (message.sequence <= connection->window_index) {
        if (message.type != TYPE_GET && message.type != TYPE_FILE)
            send_ack(connection, message.sequence);
        if (connection->recv[message.sequence].begin)
            return receive(connection);

        if (message.type == TYPE_START) {
            char type;
            unsigned int size;
            memcpy(&size, message.data, 4);
            memcpy(&type, message.data + 4, 1);

            if (type == TYPE_SCREEN)
                size++;

            message.type = type;
            message.data = (char *) calloc(size, 1);

            t_message next;
            unsigned int i = 0;

            do {
                next = receive(connection);
                memcpy(message.data + i, next.data, next.size - 3);
                free(next.data);
                i += next.size - 3;
            } while (next.type != TYPE_END);

            message.data[size - 1] = '\0';
        }

        return message;
    }
    else {
        unsigned char i;
        
        for (i = 0; i < (255 - connection->window_index); i++) {
            unsigned char index = connection->window_index + i;

            if (!connection->recv[index].begin) {
                send_nack(connection, index);
                break;
            }
        }

        if (connection->recv[message.sequence].begin == 0)
            connection->recv[message.sequence] = message;
        return receive(connection);
    }
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
        char data[5];
        memcpy(data, &size, 4);
        memcpy(data + 4, &type, 1);
        enqueue_message(connection, create_message(data, sizeof(int) + 1, TYPE_START));
        enqueue_splited(connection, message, size + 1, type);
        enqueue_message(connection, create_message("", 0, TYPE_END));
    }
    else {
        enqueue_splited(connection, message, size + 1, type);
    }
}

void print_progress(unsigned long total, unsigned long size, unsigned starttime, int reverse) {
    double p = 100 * ((total - size) / ((double) total));
    int i = 0;

    printf("\r [");
    if (!reverse) {
        for (i = 0; i < p - 1; i += 2) {
            printf("=");
        }
        printf(">");
    }

    int max = (reverse) ? 100 - p : 100;
    for (i = i; i < max; i += 2) {
        printf(" ");
    }

    if (reverse) {
        printf("<");
        for (i = 0; i < p - 1; i += 2) {
            printf("=");
        }
    }

    printf("]");

    struct timeval currtime;
    gettimeofday(&currtime, NULL);
    unsigned sent = (total - size) / 1024 / (1 + currtime.tv_sec - starttime);
    printf("\t %.1f%% %d kb/s", p, sent);
}

int send_file(t_socket *connection, const char *filename, int progress_bar) {
    unsigned long size = file_size(filename);
    enqueue_message(connection, create_message((char *) &size, sizeof(unsigned long), TYPE_FILE));
   
    t_message c;
    do {
        c = recv_message(connection);
        if (c.type == TYPE_NACK)
            handle_confirmation(connection, c);
    } while (c.type != TYPE_ACK && c.type != TYPE_ERR);

    if (c.type == TYPE_ERR) {
        if (((unsigned char) *c.data) == 3 && progress_bar)
            printf("not enought disk space\n");

        c.type = TYPE_ACK;
        handle_confirmation(connection, c);
        return 0;
    }

    struct timeval sttime;
    gettimeofday(&sttime, NULL);
    unsigned starttime = sttime.tv_sec;
    unsigned long total = size;
    FILE *fp = fopen(filename, "rb");
    while (size > MAX_FILE_QUEUE) {
        char *r = (char *) malloc(MAX_FILE_QUEUE);
        fread(r, 1, MAX_FILE_QUEUE, fp);
        size -= MAX_FILE_QUEUE;
        enqueue_splited(connection, r, MAX_FILE_QUEUE, TYPE_DATA);
        do {
            do {
                c = recv_message(connection);
                if (c.type == TYPE_NACK)
                    handle_confirmation(connection, c);
            } while (c.type != TYPE_ACK);
            handle_confirmation(connection, c);
            if (progress_bar)
                print_progress(total, size, starttime, 0);
            free(c.data);
        } while (!empty(connection->queue));
        free(r);
    }

    char *r = (char *) malloc(size);
    fread(r, 1, size, fp);
    enqueue_splited(connection, r, size, TYPE_DATA);
    size = 0;
    do {
            do {
                c = recv_message(connection);
                if (c.type == TYPE_NACK)
                    handle_confirmation(connection, c);
            } while (c.type != TYPE_ACK);
            handle_confirmation(connection, c);
            if (progress_bar)
                print_progress(total, size, starttime, 0);
            free(c.data);
    } while (!empty(connection->queue));

    if (progress_bar)
        printf("\n");

    free(r);
    fclose(fp);
    text_message(connection, TYPE_EOF, "");

    return 1;
}

void send_confirmation(t_socket *connection, const unsigned char number, const char type) {
    t_message packet = create_message("", 0, type);
    packet.sequence = number;

    char *raw_packet = generate_packet(packet);
    connection->recv[number].begin = 0;

    send_raw_data(connection, raw_packet, 5);
    free(raw_packet);
}

void send_ack(t_socket *connection, const unsigned char number) {
    connection->window_index = number + 1;
    send_confirmation(connection, number, TYPE_ACK);
}

void send_nack(t_socket *connection, const unsigned char number) {
    send_confirmation(connection, number, TYPE_NACK);
}

void send_message(t_socket *connection, const t_message message) {
    recalculate_window(connection);

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
