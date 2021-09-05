# COSC330 - Assignment 3
### Purpose
The purpose of this program is to blur a matrix of any size to any depth of bluring.
It takes a matrix file as input, applys a built-in blur function and returns a blurred matrix file.

### How to use
This program is designed to run through an MPI for distributed parallel computing.
Compile with: `mpicc -Wall -pedantic <file>.c -o <file>`
or using the makefile `make`

Then run: `mpiexec -n <dimension> <file> <dimension> <blur> <inputfile> <outputfile>`
Where:
* `<dimension>` is the size of the matrix
* `<file>` is the name of the executable
* `<blur>` is how blury the result matrix should be
* `<inputfile>` is the input matrix file
* `<outputfile>` is the output matrix file

### Author 
Tully McDonald
tmcdon26@myune.edu.au
