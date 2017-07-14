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

	// print a message rank number prefix
	rsl::tag(comm) << "i am indeed rank " << comm.rank <<"\n";

}