CC=gcc
FLAGS=-Wall -ggdb
CLIENT_EXEC=bin/client
SERVER_EXEC=bin/server
CLIENT_CFILES=client/main.c client/client.c protocol/socket.c queue/queue.c lib/sysutil/sysutil.c
SERVER_CFILES=server/main.c server/server.c protocol/socket.c queue/queue.c lib/sysutil/sysutil.c

all: c s
	
c:
	@echo " #----------------------------------------------------------------------------#"
	@echo " |                             \033[1;34mcompiling client...\033[0m                            |"
	@echo " #----------------------------------------------------------------------------#\n"
	@${CC} ${FLAGS} -o ${CLIENT_EXEC} ${CLIENT_CFILES} 2> .tmp.log || cp .tmp.log .tmp.errors
	@if test -e .tmp.errors; then echo "\033[31m" && echo "${CC} ${FLAGS} -o ${CLIENT_EXEC} ${CLIENT_CFILES} \033[1m [ ERROR ]" && cat .tmp.errors ; elif test -s .tmp.log; then echo "\033[33m" && echo "${CC} ${FLAGS} -o ${CLIENT_EXEC} ${CLIENT_CFILES} \033[1m [ WARN ]" && cat .tmp.log ; else echo "\033[32m${CC} ${FLAGS} -o ${CLIENT_EXEC} ${CLIENT_CFILES} \033[1m [ OK ]" ; fi
          
	@echo "\033[0m"

	@rm -f .tmp.errors .tmp.log 2>/dev/null

s:
	@echo " #----------------------------------------------------------------------------#"
	@echo " |                             \033[1;36mcompiling server...\033[0m                            |"
	@echo " #----------------------------------------------------------------------------#\n"
	@${CC} ${FLAGS} -o ${SERVER_EXEC} ${SERVER_CFILES} 2> .tmp.log || cp .tmp.log .tmp.errors
	@if test -e .tmp.errors; then echo "\033[31m" && echo "${CC} ${FLAGS} -o ${CLIENT_EXEC} ${SERVER_CFILES} \033[1m [ ERROR ]" && cat .tmp.errors ; elif test -s .tmp.log; then echo "\033[33m" && echo "${CC} ${FLAGS} -o ${SERVER_EXEC} ${SERVER_CFILES} \033[1m [ WARN ]" && cat .tmp.log ; else echo "\033[32m${CC} ${FLAGS} -o ${SERVER_EXEC} ${SERVER_CFILES} \033[1m [ OK ]" ; fi
          
	@echo "\033[0m"

	@rm -f .tmp.errors .tmp.log 2>/dev/null

clean:
	rm -f ${CLIENT_EXEC} ${SERVER_EXEC}
