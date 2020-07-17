.SILENT:
CC = gcc

default: lab4c_tcp.c lab4c_tls.c
	echo Building lab4c_tcp and lab4c_tls...
	${CC} -g -lm -lpthread -lmraa -lssl -lcrypto -o lab4c_tls lab4c_tls.c -Wall -Wextra
	${CC} -g -lm -lpthread -lmraa -o lab4c_tcp lab4c_tcp.c -Wall -Wextra

dist:
	tar -czvf lab4c-405118629.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c

clean:
	echo Cleaning directory...
	rm -f lab4c-405118629.tar.gz lab4c_tcp lab4c_tls