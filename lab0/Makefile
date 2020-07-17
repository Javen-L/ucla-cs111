.PHONY = lab0 check clean dist check1 check2 check3 check4 check5 check6 check7 check8 check9
CC = gcc

lab0: lab0.c
	@echo Building lab0 executable...
	@${CC} -g -o lab0 lab0.c -Wall -Wextra

check:	check1 check2 check3 check4 check5 check6 check7 check8 check9

clean:
	@echo Cleaning directory...
	@rm -f lab0 lab0-405118629.tar.gz

dist:
	@tar -czvf lab0-405118629.tar.gz lab0.c Makefile backtrace.png breakpoint.png README

check1:	#check '--input -> --output'
	@(\
	echo "CHECK:	'./lab0 --input=input_file --output=output_file'";\
	echo input test | cat > input_file;\
	./lab0 --input=input_file --output=output_file;\
	if [ $$? == 0 ];\
	then\
		echo "	exit code 0 ... OK";\
	 	diff input_file output_file | cat > temp_diff;\
		if [ $$? == 0 ];\
		then\
			echo "	file comparison ... OK";\
		else\
			echo "	file comparison ... FAIL";\
		fi;\
	else\
		echo "	exit code not 0 ... FAIL";\
	fi;\
	rm -f input_file output_file temp_diff;\
	)

check2:	#check '--input -> stdout '
	@(\
	echo "CHECK:	'./lab0 --input=input_file'";\
	echo input test | cat > input_file;\
	./lab0 --input=input_file | cat > temp_stdout;\
	if [ $$? == 0 ];\
	then\
		echo "	exit code 0 ... OK";\
	 	diff input_file temp_stdout | cat > temp_diff;\
		if [ $$? == 0 ];\
		then\
			echo "	file comparison ... OK";\
		else\
			echo "	file comparison ... FAIL";\
		fi;\
	else\
		echo "	exit code not 0 ... FAIL";\
	fi;\
	rm -f input_file temp_stdout temp_diff;\
	)

check3:	#check 'stdin -> stdout '
	@(\
	echo "CHECK:	'./lab0'";\
	echo input test | cat > temp_stdin;\
	cat temp_stdin | ./lab0 | cat > temp_stdout;\
	if [ $$? == 0 ];\
	then\
		echo "	exit code 0 ... OK";\
	 	diff temp_stdin temp_stdout | cat > temp_diff;\
		if [ $$? == 0 ];\
		then\
			echo "	file comparison ... OK";\
		else\
			echo "	file comparison ... FAIL";\
		fi;\
	else\
		echo "	exit code not 0 ... FAIL";\
	fi;\
	rm -f temp_stdin temp_stdout temp_diff;\
	)

check4:	#check	'stdin -> --output'
	@(\
	echo "CHECK:	'./lab0 --output=output_file'";\
	echo output file | cat > output_file;\
	echo input test | cat > temp_stdin;\
	cat temp_stdin | ./lab0 --output=output_file;\
	if [ $$? == 0 ];\
	then\
		echo "	exit code 0 ... OK";\
	 	diff temp_stdin output_file | cat > temp_diff;\
		if [ $$? == 0 ];\
		then\
			echo "	file comparison ... OK";\
		else\
			echo "	file comparison ... FAIL";\
		fi;\
	else\
		echo "	exit code not 0 ... FAIL";\
	fi;\
	rm -f temp_stdin output_file temp_diff;\
	)

check5:	#check missing an argument
	@(\
	echo "CHECK:	'./lab0 --input'";\
	./lab0 --input 2> temp_stderr;\
	if [ $$? == 1 ];\
	then\
		echo "	exit code 1 ... OK";\
		if [ -s temp_stderr ];\
		then\
			echo "	error message ... OK";\
			sed 's/^/\t/' temp_stderr;\
		else\
			echo " error message ... FAIL";\
		fi;\
	else\
		echo "	exit code not 1 ... FAIL";\
	fi;\
	rm -f temp_stderr;\
	)

check6:	#check invalid options
	@(\
	echo "CHECK:	'./lab0' --wrong_option";\
	./lab0 --wrong_option 2> temp_stderr;\
	if [ $$? == 1 ];\
	then\
		echo "	exit code 1 ... OK";\
		if [ -s temp_stderr ];\
		then\
			echo "	error message ... OK";\
			sed 's/^/\t/' temp_stderr;\
		else\
			echo " error message ... FAIL";\
		fi;\
	else\
		echo "	exit code not 1 ... FAIL";\
	fi;\
	rm -f temp_stderr;\
	)

check7:	#check --input=no_such_file
	@(\
	echo "CHECK:	'./lab0 --input=no_such_file'";\
	./lab0 --input=no_such_file 2> temp_stderr;\
	if [ $$? == 2 ];\
	then\
		echo "	exit code 2 ... OK";\
		if [ -s temp_stderr ];\
		then\
			echo "	error message ... OK";\
			sed 's/^/\t/' temp_stderr;\
		else\
			echo " error message ... FAIL";\
		fi;\
	else\
		echo "	exit code not 2 ... FAIL";\
	fi;\
	rm -f temp_stderr;\
	)

check8:	#check --output=no_permission_to_write
	@(\
	echo "CHECK:	'./lab0 --output=no_permission_to_write'";\
	echo test | cat > no_permission_to_write;\
	chmod u-w no_permission_to_write;\
	./lab0 --output=no_permission_to_write 2> temp_stderr;\
	if [ $$? == 3 ];\
	then\
		echo "	exit code 3 ... OK";\
		if [ -s temp_stderr ];\
		then\
			echo "	error message ... OK";\
			sed 's/^/\t/' temp_stderr;\
		else\
			echo " error message ... FAIL";\
		fi;\
	else\
		echo "	exit code not 3 ... FAIL";\
	fi;\
	rm -f no_permission_to_write temp_stderr;\
	)

check9:	#check segmentation fault & catch
	@(\
	echo "CHECK:	'./lab0 --segfault --catch'";\
	echo input test | ./lab0 --segfault --catch 2> temp_stderr;\
	if [ $$? == 4 ];\
	then\
		echo "	exit code 4 ... OK";\
		if [ -s temp_stderr ];\
		then\
			echo "	error message ... OK";\
			sed 's/^/\t/' temp_stderr;\
		else\
			echo " error message ... FAIL";\
		fi;\
	else\
		echo "	exit code not 4 ... FAIL";\
	fi;\
	rm -f temp_stderr;\
	)
