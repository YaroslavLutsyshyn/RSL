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

	// communicator contains its size and ranks of each process
	// in the form of constant variables. these can be accessed quickly
	// with virtually no overhead.

	// to avoid output overlap, here i print for only one of the tanks.
	// see tag() for better approach to printing.

	if( comm.rank == 3 )
		std::cout << "rank and size for PROCESS 3: rank " << comm.rank << "\t size " << comm.size << std::endl;

}