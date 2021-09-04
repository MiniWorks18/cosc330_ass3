#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

void printRow(int A[], int me, int dim) {
    for (int i = 0; i < dim; i++)
        printf(" %d", A[i]);
    printf("\n");
}

int parse_args(int argc, char *argv[], int *dim, int *n_depth) {
    if (argc != 3 
        || ((*dim = atoi(argv[1])) <= 0)
        || ((*n_depth = atoi(argv[2])) <= 0)) {
        fprintf(stderr,
        "Usage: mpirun -n %s dimension neighbour_depth\n", argv[0]);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int me, size, dim, sum, n_sum, row, col, n_depth, i, j, x, n;


    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (me == 0) {
        if (parse_args(argc, argv, &dim, &n_depth) == -1) {
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
    }

    // Share information from root
    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n_depth, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int A[dim][dim];
    int sDim = dim+2*n_depth;
    int superA[sDim][sDim];
    int B[dim][dim];
    int Brow[dim];
    int Arow[n_depth*2][sDim];

    if (me == 0) {
        // Define test matrix of variable dimension size
        for (i = 0; i < dim; i++) {
            for (j = 0; j < dim; j++) {
                A[i][j] = i+j*2;
                B[i][j] = 0;
            }
        }

        // Print matrix to console
        if (me == 0) {
            printf("Before:\n");
            for (i = 0; i < dim; i++)
                printRow(A[i], me, dim);
            printf("\n\n");
        }

        /* initialize supersized matrix */
        for (row = 0; row < sDim; row++) {
        for (col = 0; col < n_depth; col++) {
            // Set frame around superA of 0s
            superA[row][col] = superA[row][sDim-col-1] = 0;
            superA[col][row] = superA[sDim-col-1][row] = 0;
        }
        }

        // Place A into superA
        for (i = 0; i < dim; i++) {
            for (j = 0; j < dim; j++)
                superA[i+n_depth][j+n_depth] = A[i][j];
        }

        printf("SuperA: \n");
        for (i = 0; i < sDim; i++) {
            for (j = 0; j < sDim; j++)
            printf(" %d", superA[i][j]);
            printf("\n");
        }
    }

    for (i = 0; i < n_depth*2+1; i++) {
        if (MPI_Scatter(&superA[i], sDim, MPI_INT,
                        &Arow[i], sDim, MPI_INT,
                        0, MPI_COMM_WORLD) != MPI_SUCCESS) {
                            fprintf(stderr, "Scattering of A failed\n");
                            MPI_Finalize();
                            exit(EXIT_FAILURE);
                        }
    }

    printf("Process %d got:\n", me);
    for (i = 0; i < 2*n_depth+1; i++) {
        for (j = 0; j < sDim; j++) 
            printf(" %d", Arow[i][j]);
        printf("\n");
    }


    x = n_depth;
    // For each cell in our designated row to calculate
    for (int y = n_depth, j = 0; y < n_depth+dim; y++, j++) {
        sum = 0;
        // For each layer of neighbours
        for (n = 1; n <= n_depth; n++) {
            n_sum = 0;
            // Sum top and bottom neighbours
            for (i = y-n; i <= y+n; i++)
                n_sum += Arow[x-n][i] + Arow[x+n][i];
            // Sum left and right neighbours
            for (i = x-n+1; i <= x+n-1; i++) 
                n_sum += Arow[i][y-n] + Arow[i][y+n];
            // Add weighted sum to total sum
            sum += n_sum/n;
        }
        // Print total sum
        // printf("Arow[%d][%d] (%d) sum: %d\n", n_depth, x, Arow[n_depth][x], sum);
        printf("Brow[%d] = %d\n", j, sum);
        Brow[j] = sum;
    }
    
    if (MPI_Gather(&Brow[0], dim, MPI_INT,
                    &B[0][0], dim, MPI_INT,
                    0, MPI_COMM_WORLD) != MPI_SUCCESS) {
                        fprintf(stderr, "Gathering of Product failed\n");
                        MPI_Finalize();
                        exit(EXIT_FAILURE);
                    }

   
    if (me == 0) {
        printf("After:\n");
        for (i = 0; i < dim; i++) {
            for (j = 0; j < dim; j++) 
                printf(" %d", B[i][j]);
            printf("\n");
        }
        printf("\n\n");
    }


    MPI_Finalize();
    return 0;
}