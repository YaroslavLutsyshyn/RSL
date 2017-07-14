/*
Test the tag functionality for the communicator class.
Each stdout message can be tagged with rank and nanosec imestamp.
Here, there is no time just rank.
*/

#include <iostream>
#include "rsl.h"

int main() {

	// initialize MPI
	rsl::mpi_env mpi;

	// create an instance of the communicator class
	// no arguments to constructor, this will be MPI_COMM_WORLD
	rsl::mpi_communicator comm;  

	// syncronizes clocks across the ranks, via an attempt to measure 
	// clock differences. this should allow for a uniform timestamp across ranks
	comm.set_clocks();


	// print a message with rank number and nanosecond timestamp prefix
	// this in principle allows to sort evens accross different ranks in
	// the order of execution.
	// timestamp is produced because of an earlier call to set_clocks() on
	// this communicator.
	rsl::tag(comm) << "i am indeed rank " << comm.rank <<"\n";

}