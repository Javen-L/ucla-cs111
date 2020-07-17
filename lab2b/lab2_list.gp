#! /usr/bin/gnuplot
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#    8. avg time per wait-for-lock

# general plot parameters
set terminal png
set datafile separator ","

# TEST1, lab2b_1.png
set title "1. Throughput vs # of Threads"
set xlabel "# of Threads"
set xrange [0.75:]
set ylabel "Throughput [/s]"
set logscale y
set output 'lab2b_1.png'
set key left bottom
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'Mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'Spin-lock' with linespoints lc rgb 'green'

# lab2b_2.png
set title "2. Wait Time, Cost per Operation vs # of Threads"
set xlabel "# of Threads"
set xrange [0.75:]
set ylabel "Time [ns]"
set logscale y
set output 'lab2b_2.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'wait-for-lock time' with linespoints lc rgb 'brown', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'time per operation' with linespoints lc rgb 'orange'

# TEST2, lab2b_3.png
set title "3. Iterations & Threads That Run Successfully"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:20]
set ylabel "Iterations"
set logscale y 2
set output 'lab2b_3.png'

plot \
     "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'no protection' with points lc rgb 'red', \
     "< grep list-id-m lab2b_list.csv" using ($2):($3) \
	title 'mutex' with points lc rgb 'blue', \
     "< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'spin-lock' with points pointtype 4 lc rgb 'green'


# TEST3, lab2b_4.png
set title "1. Throughput vs # of Threads (Mutex)"
set xlabel "# of Threads"
set xrange [0.75:]
set ylabel "Throughput [/s]"
set logscale y
set output 'lab2b_4.png'
set key left bottom

plot \
     "< grep -e 'list-none-m,.2\\?,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'list=1' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'list=4' with linespoints lc rgb 'green',\
     "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'list=8' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'list=16' with linespoints lc rgb 'red'


# TEST4, lab2b_5.png
set title "1. Throughput vs # of Threads (Spin-lock)"
set xlabel "# of Threads"
set xrange [0.75:]
set ylabel "Throughput [/s]"
set logscale y
set output 'lab2b_5.png'
set key left bottom

plot \
     "< grep -e 'list-none-s,.2\\?,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'list=1' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'list=4' with linespoints lc rgb 'green',\
     "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'list=8' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'list=16' with linespoints lc rgb 'red'