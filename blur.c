/*
Purpose:
This program is designed to take a matrix file as input
and produce a blured version as output.

How to use:
* Compile using mpicc
    * make
* Run on a MPI system like so:
    * mpiexec -n <dimensions> <filename> <dimensions> <blur> 
        <inputfile> <output file>

    * e.g mpiexec -n 4 blur 4 2 in.mat out.mat
    
    Where:
    * <dimensions> is the dimensions of the array
    * <filename> is the name of the executable
    * <blur> is how blury the results should be

Author:
Tully McDonald
tmcdon26@gmail.com
*/
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "matrix.c"

int parse_args(int argc, char *argv[], int *dim, int *n_depth, int *fd) {
    if (argc != 5 
        || ((*dim = atoi(argv[1])) <= 0)
        || ((*n_depth = atoi(argv[2])) <= 0)
        || ((fd[0] = open(argv[3], O_RDONLY)) == -1) 
        || ((fd[1] = open(argv[4], O_WRONLY | O_CREAT, 0666)) == -1)) {
        fprintf(stderr,
        "Usage: mpirun -n %s dimension neighbour_depth inputfile outputfile\n",
         argv[0]);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int me, size, dim, sum, n_sum, row, col, n_depth, i, j, x, n, fd[2];

    // Initialise communicator
    if (MPI_Init(&argc, &argv)  != MPI_SUCCESS) {
        fprintf(stderr, "Failed on mpi initalisation\n");
        MPI_Finalize();
        exit(EXIT_FAILURE);
    } 
    if (MPI_Comm_rank(MPI_COMM_WORLD, &me) != MPI_SUCCESS) {
        fprintf(stderr, "Failed to get process rank\n");
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if (MPI_Comm_size(MPI_COMM_WORLD, &size) != MPI_SUCCESS) {
        fprintf(stderr, "Failed to get communicator\n");
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (me == 0) {
        if (parse_args(argc, argv, &dim, &n_depth, fd) == -1) {
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
    }

    // Share information from root
    if (MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD) != MPI_SUCCESS ||
        MPI_Bcast(&n_depth, 1, MPI_INT, 0, MPI_COMM_WORLD)) {
            fprintf(stderr, "Failed to share data\n");
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }

    int A[dim+1][dim+1], sDim = dim+2*n_depth, superA[sDim][sDim];
    int B[dim+1][dim+1], Arow[n_depth*2][sDim];

    /* 
    This is the weirdest behaviour i've ever seen.
    If result_row is declared as [dim+1] (what i first tried)
    the values within Arow[][] suddenly change when we place
    sum inside result_row after computing the sum
    */
    int result_row[1000000];

    if (me == 0) {
        // Define test matrix of variable dimension size
        for (i = 1; i < dim+1; i++) {
            if (get_row(fd[0], dim, i, &A[i][1]) == -1) {
                fprintf(stderr, "Initialization of A failed\n");
                MPI_Finalize();
                exit(EXIT_FAILURE);
            }
        }
        x = n_depth;

        /* initialize supersized matrix */
        for (row = 0; row < sDim; row++) {
            for (col = 0; col < n_depth; col++) {
                // Set frame around superA of 0s
                superA[row][col] = superA[row][sDim-col-1] = 0;
                superA[col][row] = superA[sDim-col-1][row] = 0;
            }
        }

        // Place A into superA
        for (i = 1; i < dim+1; i++) {
            for (j = 1; j < dim+1; j++)
                superA[i+n_depth-1][j+n_depth-1] = A[i][j];
        }
    }

    // Distribute the matrix to the processes
    for (i = 0; i < n_depth*2+1; i++) {
        if (MPI_Scatter(&superA[i], sDim, MPI_INT,
                        &Arow[i], sDim, MPI_INT,
                        0, MPI_COMM_WORLD) != MPI_SUCCESS) {
                            fprintf(stderr, "Scattering of A failed\n");
                            MPI_Finalize();
                            exit(EXIT_FAILURE);
                        }
    }

    x = n_depth;
    // For each cell in our designated row to calculate
    for (int y = n_depth, j = 1; y < n_depth+dim; y++, j++) {
        sum = 0;
        // For each layer of neighbours
        for (n = 1; n <= n_depth; n++) {
            n_sum = 0;
            // Sum top and bottom neighbours
            for (i = y-n; i <= y+n; i++) {
                n_sum += Arow[x-n][i];
                n_sum += Arow[x+n][i];
            }
            // Sum left and right neighbours
            for (i = x-n+1; i <= x+n-1; i++) {
                n_sum += Arow[i][y-n];
                n_sum += Arow[i][y+n];
            }
            sum += n_sum/n;
        }
        result_row[j] = sum;
    }
    
    // Gather the results from the processes
    if (MPI_Gather(&result_row[0], dim+1, MPI_INT,
                    &B[1][0], dim+1, MPI_INT,
                    0, MPI_COMM_WORLD) != MPI_SUCCESS) {
                        fprintf(stderr, "Gathering of Product failed\n");
                        MPI_Finalize();
                        exit(EXIT_FAILURE);
                    }

    // Write result matrix to outfile
    if (me == 0) {
        for (i = 1; i < dim+1; i++) {
            if (set_row(fd[1], dim, i, &B[i][1]) == -1) {
                fprintf(stderr, "Writing of matrix outfile failed\n");
                MPI_Finalize();
                exit(EXIT_FAILURE);
            }
        }
    }

    MPI_Finalize();
    exit(EXIT_SUCCESS);
}