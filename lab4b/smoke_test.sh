#!/bin/bash

echo -n "checking if '--period' supported ... "
timeout 1 ./lab4b --period=1 > stdout
if [ $? == 124 ]
then
	echo "PASS"
else
    echo "FAIL"
fi

echo -n "checking if '--scale' supported ... "
timeout 1 ./lab4b --scale=C > stdout
if [ $? == 124 ]
then
	echo "PASS"
else
    echo "FAIL"
fi

echo -n "checking if '--log' supported ... "
timeout 1 ./lab4b --log=logfile
if [ -f logfile ]
then
	echo "PASS"
else
	echo "FAIL"
fi

echo -n "checking if invlalid options detected ... "
timeout 1 ./lab4b --invalid 2> stderr
if [ $? == 1 ]
then
	echo "PASS"
    echo "  "$(cat stderr)""
else
    echo "FAIL"
fi

echo -n "checking if stdin commands supported ... "
./lab4b --log=logfile << EOF
PERIOD=2
STOP
START
SCALE=C
LOG test
OFF
EOF

if [ $? == 0 ]
then
	echo " PASS"
else
    echo " FAIL"
fi

echo -n "checking logfile content ... "
err=0
for command in PERIOD=2 STOP START SCALE=C "LOG test" OFF
	do
        grep -q "$command" logfile
		if [ $? == 1 ]
		then
            if [ $err == 0 ]
            then
		    	echo "FAIL"
                let err+=1
            fi
			echo "  '$command' NOT FOUND"
        fi
done

	if [ $err == 0 ]
	then
		echo "PASS"
	fi

rm -f logfile stdout stderr