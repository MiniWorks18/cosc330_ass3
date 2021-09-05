#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "matrix.c"

int main(int argc, char *argv[]) {
    int fd, dim;
    if (((fd = open(argv[1], O_RDONLY)) == -1)
        || ((dim = atoi(argv[2])) <= 0)) {
        fprintf(stderr, "Usage: mpirun -n dimension %s matFile", argv[0]);
        return(EXIT_FAILURE);
    }

    int A[dim][dim];

    for (int i = 1; i < dim+1; i++) {
        if (get_row(fd, dim, i, &A[i][1]) == -1) {
            fprintf(stderr, "Failed to read file\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 1; i < dim+1; i++) {
        for (int j = 1; j<dim+1; j++) {
            printf(" %d", A[i][j]);
        }
        printf("\n");
    }
    return 0;
}