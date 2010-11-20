CC=gcc
FLAGS=-Wall -ggdb
CLIENT_EXEC=bin/client
SERVER_EXEC=bin/server
CLIENT_CFILES=client/main.c client/client.c protocol/socket.c
SERVER_CFILES=server/main.c server/server.c protocol/socket.c

all: c s
	
c:
	@echo "#--------------------------------------------------------------------------#"
	@echo "|                            \033[1mcompiling client\033[0m                              |"
	@echo "#--------------------------------------------------------------------------#"
	${CC}  ${FLAGS} -o ${CLIENT_EXEC} ${CLIENT_CFILES}

s:
	@echo "#--------------------------------------------------------------------------#"
	@echo "|                            \033[1mcompiling server\033[0m                              |"
	@echo "#--------------------------------------------------------------------------#"
	${CC}  ${FLAGS} -o ${SERVER_EXEC} ${SERVER_CFILES}

clean:
	rm -f ${CLIENT_EXEC} ${SERVER_EXEC}
