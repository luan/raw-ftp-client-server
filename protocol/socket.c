#include "socket.h"

int socket_create(char* device) {
    int raw_socket, deviceid;

    struct ifreq ifr;
    struct sockaddr_ll sll;
    struct packet_mreq mr;

    raw_socket = socket(PF_PACKET, SOCK_RAW, 0);
    if (raw_socket == -1)
        exit(0);

    // get device ID
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

    // modo promiscuo
    memset(&mr, 0, sizeof(mr));

    mr.mr_ifindex = deviceid;
    mr.mr_type    = PACKET_MR_PROMISC;

    if (setsockopt(raw_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
        exit(3);

    return raw_socket;
}

