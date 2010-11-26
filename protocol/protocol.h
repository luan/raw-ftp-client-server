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

/*
    #------#--------------------------------#
    # Tipo | descrição                      #
    #------#--------------------------------#
    #  L   # ls                             #
    #  Y   # AcK  <número de sequencia>     #
    #  N   # NacK <número de sequencia>     #
    #  X   # mostrar na tela                #
    #  Z   # fim de arquivo                 #
    #  C   # cd <caminho do diretório>      #
    #  E   # erro <codigo do erro>          #
    #  P   # put <nome do arquivo>          #
    #  F   # descritor do arquivo <tamanho> #
    #  D   # dados                          #
    #  G   # get <nome do arquivo>          #
    #------#--------------------------------#
*/

#define   TYPE_LS     'L'
#define   TYPE_CD     'C'
#define   TYPE_ERR    'E'
#define   TYPE_PUT    'P'
#define   TYPE_GET    'G'
#define   TYPE_FILE   'F'
#define   TYPE_DATA   'D'
#define   TYPE_EOF    'Z'
#define   TYPE_ACK    'Y'
#define   TYPE_NACK   'N'
#define   TYPE_SCREEN 'X'
#define   TYPE_START  'A'
#define   TYPE_END    'B'

#endif
