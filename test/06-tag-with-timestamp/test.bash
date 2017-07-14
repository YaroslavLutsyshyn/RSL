#!/bin/bash

if [ ! -f exec ]
	then
	echo failed, no executable
	exit 1
fi

mpirun -n 4 ./exec > log  2> err

if [ -s err ]
	then
	echo failed, stderr present
	exit 1
fi

if [ ! -s log ]
	then
	echo failed, stdout missing
	exit 1
fi

###############################################################################
# Now specific to this executable

cat log | grep indeed | sort | sed 's/:.*]/ XXX /' > log2

if ! cmp -s log2 data/reference.log2 
	then
	echo failed, wrong output
	exit 1
fi

# zero seconds from program start

cat log | grep indeed | sort | sed -e 's/^[^|]*|//' -e 's/|.*//' -e 's/ *//g' > log3
if ! cmp -s log3 data/reference.log3
	then
	echo "failed, wrong output, too much time at the beginning"
	exit 1
fi

# execution time in microseconds should not be too different

cat log | grep indeed | sort | sed -e 's/^[^|]*|//'  -e 's/^[^|]*|//' -e 's/|.*//' -e 's/ *//g' > log4
cat log4 | awk 'BEGIN{x=-1;m=0}{if(x<0)x=$1;d=($1-x);if(d<0)d=-d;if(d>m)m=d;}END{print (m<1000)}' > log5

if ! cmp -s log5 data/reference.log5
	then
	echo "failed, wrong output, too much time between processes (over 1ms)"
	exit 1
fi


###############################################################################
# Made it

echo passed $(pwd)
exit 0
