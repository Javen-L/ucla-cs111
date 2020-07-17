.PHONY = lab1b client server clean dist
CC = gcc

lab1b: lab1b-client.c lab1b-server.c
	@echo Building lab1b-client and lab1b-server...
	@${CC} -g -lz -o lab1b-client lab1b-client.c -Wall -Wextra
	@${CC} -g -lz -o lab1b-server lab1b-server.c -Wall -Wextra

client: lab1b-client.c
	@echo Building lab1b-client...
	@${CC} -g -lz -o lab1b-client lab1b-client.c -Wall -Wextra

server: lab1b-server.c
	@echo Building lab1b-server...
	@${CC} -g -lz -o lab1b-server lab1b-server.c -Wall -Wextra

clean:
	@echo Cleaning directory...
	@rm -f lab1b-client lab1b-server lab1b-405118629.tar.gz

dist:
	@tar -czvf lab1b-405118629.tar.gz lab1b-client.c lab1b-server.c Makefile README
