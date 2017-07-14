/*
Test communicator class with MPI_COMM_WORLD.
Notice basic printing operator for the communicator.
This has to be sorted for the test since the print order is not defined.
*/

#include <iostream>
#include "rsl.h"

int main() {

	// initialize MPI
	rsl::mpi_env mpi;

	// create an instance of the communicator class
	// no arguments to constructor, this will be MPI_COMM_WORLD
	rsl::mpi_communicator comm;  

	std::cout << comm << std::endl;

}