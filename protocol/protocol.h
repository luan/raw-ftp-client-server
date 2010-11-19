#ifndef _PROTOCOL_
#define _PROTOCOL_

typedef struct {
    unsigned begin:8;
    unsigned size:8;
    unsigned sequence:8;
    unsigned type:8;
    char *data;
    unsigned parity:8;
} t_message;

#endif
