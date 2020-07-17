.PHONY = build tests graphs dist clean tests
CC = gcc

build: lab2_add.c lab2_list.c
	@echo Building lab2_add and lab2_list...
	@${CC} -g -lpthread -o lab2_add lab2_add.c -Wall -Wextra
	@${CC} -g -lpthread -o lab2_list lab2_list.c SortedList.c -Wall -Wextra

tests: test_add test_list

test_add: test00 test1 test2 test3 test4 test5 test6

test_list: test01 test7 test8 test9 test10 test11

graphs:
	@./lab2_add.gp
	@./lab2_list.gp

dist: build tests graphs
	@tar -czvf lab2a-405118629.tar.gz Makefile README lab2_add.c lab2_list.c SortedList.c SortedList.h *.png *.csv *.gp

clean:
	@echo Cleaning directory...
	@rm -f lab2a-405118629.tar.gz lab2_add lab2_list

test00: # Create/clean lab2_add.csv
	@echo -n| cat >lab2_add.csv

test01: # Create/clean lab2_list.csv
	@echo -n| cat >lab2_list.csv

test1: # Q2.1.1
	@for t in 2 4 8 12; do\
		for i in 100 1000 10000 100000; do\
			./lab2_add --iterations=$$i --threads=$$t >> lab2_add.csv;\
		done;\
	done

test2: # lab2_add-1.png
	@for t in 2 4 8 12; do\
		for i in 10 20 40 80 100 1000 10000 100000; do\
			./lab2_add --iterations=$$i --threads=$$t --yield >> lab2_add.csv;\
		done;\
	done

test3: # lab2_add-2.png, Q2.1.2
	@for t in 2 8; do\
		for i in 100 1000 10000 100000; do\
			./lab2_add --iterations=$$i --threads=$$t >> lab2_add.csv;\
			./lab2_add --iterations=$$i --threads=$$t --yield >> lab2_add.csv;\
		done;\
	done

test4: # lab2_add-3.png, Q2.1.3
	@for i in 1 10 100 1000 10000 100000 1000000; do\
		./lab2_add --iterations=$$i >> lab2_add.csv;\
	done

test5: # lab2_add-4.png
	@for t in 2 4 8 12; do\
		./lab2_add --iterations=10000 --threads=$$t --yield --sync=m >> lab2_add.csv;\
		./lab2_add --iterations=1000 --threads=$$t --yield --sync=s >> lab2_add.csv;\
		./lab2_add --iterations=10000 --threads=$$t --yield --sync=c >> lab2_add.csv;\
	done

test6: # lab2_add-5.png, Q2.1.4
	@for t in 1 2 4 8 12; do\
		./lab2_add --iterations=10000 --threads=$$t >> lab2_add.csv;\
		./lab2_add --iterations=10000 --threads=$$t --sync=m >> lab2_add.csv;\
		./lab2_add --iterations=10000 --threads=$$t --sync=s >> lab2_add.csv;\
		./lab2_add --iterations=10000 --threads=$$t --sync=c >> lab2_add.csv;\
	done

test7: # lab2_list-1.png
	@for i in 10 100 1000 10000 20000; do\
		./lab2_list --iterations=$$i >> lab2_list.csv;\
	done

test8: 
	@-./lab2_list --iterations=1 --threads=2 >> lab2_list.csv
	@-./lab2_list --iterations=10 --threads=2 >> lab2_list.csv
	@-./lab2_list --iterations=100 --threads=2 >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=2 >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=4 >> lab2_list.csv
	@-./lab2_list --iterations=10 --threads=4 >> lab2_list.csv
	@-./lab2_list --iterations=100 --threads=4 >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=4 >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=8 >> lab2_list.csv
	@-./lab2_list --iterations=10 --threads=8 >> lab2_list.csv
	@-./lab2_list --iterations=100 --threads=8 >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=8 >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=12 >> lab2_list.csv
	@-./lab2_list --iterations=10 --threads=12 >> lab2_list.csv
	@-./lab2_list --iterations=100 --threads=12 >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=12 >> lab2_list.csv

test9: # lab2_list-2.png.
	@-./lab2_list --iterations=1 --threads=2 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=2 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=2 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=2 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=2 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=2 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=4 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=4 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=4 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=4 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=4 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=4 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=8 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=8 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=8 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=8 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=8 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=8 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=12 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=12 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=12 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=12 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=12 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=i >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=2 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=2 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=2 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=2 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=2 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=2 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=4 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=4 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=4 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=4 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=4 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=4 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=8 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=8 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=8 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=8 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=8 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=8 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=12 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=12 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=12 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=12 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=12 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=d >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=2 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=2 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=2 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=2 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=2 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=2 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=4 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=4 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=4 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=4 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=4 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=4 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=8 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=8 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=8 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=8 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=8 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=8 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=12 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=12 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=12 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=12 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=12 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=il >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=2 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=2 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=2 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=2 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=2 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=2 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=4 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=4 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=4 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=4 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=4 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=4 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=8 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=8 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=8 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=8 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=8 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=8 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=1 --threads=12 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=2 --threads=12 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=4 --threads=12 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=8 --threads=12 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=16 --threads=12 --yield=dl >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=dl >> lab2_list.csv

test10: # lab2_list-3.png
	@-./lab2_list --iterations=32 --threads=12 --yield=i --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=i --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=d --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=d --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=il --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=il --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=dl --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=32 --threads=12 --yield=dl --sync=s >> lab2_list.csv

test11: #lab2_list-4.png
	@-./lab2_list --iterations=1000 --threads=1 --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=1 --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=2 --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=2 --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=4 --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=4 --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=8 --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=8 --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=12 --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=12 --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=16 --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=16 --sync=s >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=24 --sync=m >> lab2_list.csv
	@-./lab2_list --iterations=1000 --threads=24 --sync=s >> lab2_list.csv