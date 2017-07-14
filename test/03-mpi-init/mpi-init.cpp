/*
Test mpi initialization routines
*/

#include <iostream>
#include "rsl.h"

int main() {

	rsl::mpi_env mpi;

	int x {4};
	std::cout << x << std::endl;

}