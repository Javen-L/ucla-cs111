.SILENT:
CC = gcc

default: lab2_list.c
	echo Building lab2_list...
	${CC} -g -lpthread -lprofiler -o lab2_list lab2_list.c SortedList.c -Wall -Wextra

tests: test0 test1 test2 test3

profile:
	echo -n| cat >profile.out
	env CPUPROFILE=lab2_list.prof ./lab2_list --iterations=1000 --threads=12 --sync=s
	pprof --text ./lab2_list lab2_list.prof >> profile.out
	pprof --list=thread_routine ./lab2_list lab2_list.prof >> profile.out
	rm -f lab2_list.prof

graphs:
	./lab2_list.gp

dist: default tests
	tar -czvf lab2b-405118629.tar.gz Makefile README lab2_list.c SortedList.c SortedList.h lab2b_*.png lab2b_list.csv lab2_list.gp profile.out

clean:
	echo Cleaning directory...
	rm -f lab2b-405118629.tar.gz lab2_list

test0: # Create/clean lab2b_list.csv
	echo -n| cat >lab2b_list.csv

test1: # lab2b_1.png, lab2b_2.png, lab2b_4.png, lab2b_5.png
	-./lab2_list --iterations=1000 --threads=1 --sync=m >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=2 --sync=m >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=4 --sync=m >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=8 --sync=m >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=12 --sync=m >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=16 --sync=m >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=24 --sync=m >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=1 --sync=s >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=2 --sync=s >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=4 --sync=s >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=8 --sync=s >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=12 --sync=s >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=16 --sync=s >> lab2b_list.csv
	-./lab2_list --iterations=1000 --threads=24 --sync=s >> lab2b_list.csv

test2: # lab2b_3.png
	-./lab2_list --threads=1 --iterations=1 --lists=4 --yield=id  >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv

test3: # lab2b_4.png, lab2b_5.png
	-./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv