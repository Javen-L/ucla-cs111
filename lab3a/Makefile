.SILENT:
CC = gcc

default: lab3a.c
	echo building lab3a...
	${CC} -g -lm -o lab3a lab3a.c -Wall -Wextra
dist:
	tar -czvf lab3a-405118629.tar.gz Makefile README lab3a.c ext2_fs.h

clean:
	echo Cleaning directory...
	rm -f lab3a-405118629.tar.gz lab3a