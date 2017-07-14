
.PHONY: all
all:
	make --no-print-directory clean
	mpic++ -std=c++11 -I '../../include' -L '../../lib/' mpi-init.cpp -lrsl -o exec
	-bash test.bash

.PHONY: clean
clean:	
	-@rm -f log err exec