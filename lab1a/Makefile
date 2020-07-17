.PHONY = lab1a clean dist
CC = gcc

lab1a: lab1a.c
	@echo Building lab1a executable...
	@${CC} -g -o lab1a lab1a.c -Wall -Wextra

clean:
	@echo Cleaning directory...
	@rm -f lab1a lab1a-405118629.tar.gz

dist:
	@tar -czvf lab1a-405118629.tar.gz lab1a.c Makefile README
