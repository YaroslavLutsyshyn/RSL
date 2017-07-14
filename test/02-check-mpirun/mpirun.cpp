/*
Tests mpirun. This is supposed to be ran be test.bash in four instances and thus print four numerals "4"
*/

#include <iostream>
#include "rsl.h"

int main() {

	int x {4};

	std::cout << x << std::endl;
}