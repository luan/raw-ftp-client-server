CC=gcc
FLAGS=-Wall
EXEC=../bin/server
CFILES=main.o server.o ../protocol/socket.o

all: protocol ${CFILES}
	${CC} ${FLAGS} -o ${EXEC} ${CFILES}

protocol:
	make -C ../protocol

$<:
	${CC} ${FLAGS} -c $*.c

clean:
	rm -f *.o *.swp ${EXEC}
