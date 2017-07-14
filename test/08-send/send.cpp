/*
Test the tag functionality for the communicator class.
Each stdout message can be tagged with rank and nanosec imestamp.
Here, there is no time just rank.
*/

#include <iostream>
#include <string>

#include "rsl.h"

int main() {

	// initialize MPI
	rsl::mpi_env mpi;

	// create an instance of the communicator class
	// no arguments to constructor, this will be MPI_COMM_WORLD
	rsl::mpi_communicator comm;  

	// creating an instance of send class initiates non-blocking send.
	// threads that are neither senders nor receivers ignore all calls.
	// destruction of this object on sender or receiver forces the request 
	// wait on that process. call use() method to get data.

	int val [3];

	if( comm.rank == 0 ) {
		for(int i=0; i<3; ++i)
			val [i] = 11+i;
	}
	else if( comm.rank == 1 ){
		for(int i=0; i<3; ++i)
			val [i] = 77 + i;	
	}
	else {
		for(int i=0; i<3; ++i)
			val [i] = 102 + i;	
	} 
	
	// notice this is called on ALL RANKS.
	// enough to call on sender and receiver, here sending from 1 to 2
	// tag can be provided as an additional optional argument, defaults to zero.

	rsl::mpi_send<int,3> s(comm,val,1,2); 


	// now receive the data
	// this only causes work on receiver

	s.use(val);

	if( comm.rank ==2 )
		rsl::tag(comm) << val[0] << "\t" << val[1] << "\t" << val[2] << "\n"; 


	// notice that immediate use is possible
	// here i create an anonymous object and call .use() on it right away

	rsl::mpi_send<int,3>{comm,val,0,3}.use(val); 

	if( comm.rank ==3 )
		rsl::tag(comm) << val[0] << "\t" << val[1] << "\t" << val[2] << "\n"; 

	// here again will send one value from root to all ranks, and receive immediately

	double x;

	if( comm.rank == 0 )
		x = 0.55;
	else 
		x = 0.;

	for(int r=1; r<comm.size; ++r ) {
		rsl::mpi_send<double,1>{comm,&x,0,r}.use(&x); 
	}

	rsl::tag(comm) << x << "\n";
}

