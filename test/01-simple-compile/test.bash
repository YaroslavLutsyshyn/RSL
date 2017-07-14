#!/bin/bash

if [ ! -f exec ]
	then
	echo failed, no executable
	exit 1
fi

./exec > log  2> err

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

v=$(cat log)

if [ $v != "4" ]
	then
	echo failed, wrong output
	exit 1
fi

###############################################################################
# Made it

echo passed $(pwd)
exit 0
