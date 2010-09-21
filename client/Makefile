CC=gcc
FLAGS=-Wall
EXEC=../bin/client
CFILES=main.o client.o ../protocol/socket.o

all: protocol ${CFILES}
	${CC} ${FLAGS} -o ${EXEC} ${CFILES}

protocol:
	make -C ../protocol

$<:
	${CC} ${FLAGS} -c $*.c

clean:
	rm -f *.o *.swp ${EXEC}
