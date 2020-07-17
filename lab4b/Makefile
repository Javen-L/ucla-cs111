.SILENT:
CC = gcc

default: lab4b.c
	echo Building lab4b...
	${CC} -g -lm -lpthread -lmraa -o lab4b lab4b.c -Wall -Wextra

check: 
	chmod u+x ./smoke_test.sh
	./smoke_test.sh
	
dist:
	tar -czvf lab4b-405118629.tar.gz Makefile README lab4b.c smoke_test.sh

clean:
	echo Cleaning directory...
	rm -f lab4b-405118629.tar.gz lab4b