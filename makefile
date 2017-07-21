

all_source_and_header_files:=src/rsl.cpp 
all_source_and_header_files+=include/rsl.h 
all_source_and_header_files+=include/rsl_mpi_send.h
all_source_and_header_files+=include/rsl_serializer.h
all_library_files:=lib/librsl.a

all: use

$(all_library_files): $(all_source_and_header_files)
	make lib

.PHONY:lib
lib: $(all_source_and_header_files)
	mpic++ -std=c++11 -I include src/rsl.cpp -c -o obj/rsl.o
	ar rvs lib/librsl.a obj/rsl.o

.PHONY: clean
clean:
	-rm -f lib/*
	-rm -f obj/*

.PHONY: test
test: $(all_library_files)
	@echo && $(MAKE) -C test/01-simple-compile
	@echo && $(MAKE) -C test/02-check-mpirun
	@echo && $(MAKE) -C test/03-mpi-init
	@echo && $(MAKE) -C test/04-communicator
	@echo && $(MAKE) -C test/05-tag-no-time
	@echo && $(MAKE) -C test/06-tag-with-timestamp
	@echo && $(MAKE) -C test/07-communicator-rank-and-size
	@echo && $(MAKE) -C test/08-send

 
.PHONY: testclean
testclean:
	for d in test/* ; do (cd $d; pwd; make clean) ; done


.PHONY: use
use: $(all_library_files)
	@echo && $(MAKE) -C use
