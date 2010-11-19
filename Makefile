all: c s
	
c:
	make -C client

s:
	make -C server

clean:
	make clean -C client
	make clean -C server
	make clean -C protocol
