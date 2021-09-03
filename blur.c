#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#define DIM 5

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
    int me, size, dim, sum, row, r, col, c, n_depth, l;


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
    // int B[dim][dim];

    // Define test matrix of variable dimension size
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            A[i][j] = i+j*2;
            // B[i][j] = 0;
        }
    }

    // Print matrix to console
    if (me == 0) {
        printf("Before:\n");
        for (int i = 0; i < dim; i++)
            printRow(A[i], me, dim);
        printf("\n\n");
    }

    if (me == 0) {
        row = 2;
        col = 2;
        r = row-n_depth;
        c = col-n_depth;
        l = n_depth*2+1; // Length of neighbours dimension
        sum = 0;
        for (int i = 0; i < l; i++) {
            if ((r+i) >= 0 && (r+i) <= dim-1 && c >= 0 && c <= dim-1) {
                printf("Left[%d][%d]: %d\n",r+i, c, A[r+i][c]);
                sum += A[r+i][c]; // Fetch neighbours left side
            }
            if ((r+i) >= 0 && (r+i) <= dim-1 && (c+l-1) >= 0 && (c+l-1) <= dim-1) {
                printf("Right[%d][%d]: %d\n",r+i, c+l-1, A[r+i][c+l-1]);
                sum += A[r+i][c+l-1]; // Fetch neighbours right side
            }
            
            if (i > 0 && i < l-1) {
                if (r >= 0 && r <= dim-1 && c+i >= 0 && c+i <= dim-1) {
                    printf("Top[%d][%d]: %d\n", r, c+i, A[r][c+i]);
                    sum += A[r][c+i]; // Fetch neighbours top
                }
                if (r+l-1 >= 0 && r+l-1 <= dim-1 && c+i >= 0 && c+i <= dim-1) {
                    printf("Bottom[%d][%d]: %d\n", r+l-1, c+i, A[r+l-1][c+i]);
                    sum += A[r+l-1][c+i]; // Fetch neighbours bottom
                }
            }
        }
        sum = sum/n_depth;
        printf("Blur of A[%d][%d](%d) is %d\n", row, col, A[row][col], sum);
    }

    MPI_Finalize();
    return 0;
}