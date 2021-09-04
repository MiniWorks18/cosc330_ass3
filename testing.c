#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#define DIM 3

int parse_args(int argc, char *argv[], int *dim)
{
    if (argc != 2 || ((*dim = atoi(argv[1])) <= 0))
    {
        fprintf(stderr,
                "Usage: mpirun -n %s dimension\n", argv[0]);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int me, size, dim;
    int n_depth = 1;

    // int chain[dim * dim];
    if (parse_args(argc, argv, &dim) == -1)
    {
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    int A[dim][dim];
    int superA[n_depth + dim][n_depth + dim];
    int B[n_depth + dim][n_depth + dim];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (me == 0)
    {

        for (int i = 0; i < dim; i++)
        {
            for (int j = 0; j < dim; j++)
            {
                A[i][j] = i + 2 * j;
                B[i][j] = 0;
            }
        }
    }

    for (int row = 0; row < 2 * n_depth + dim; row++)
        superA[row][0] = superA[row][2 * n_depth + dim] = 0;
    for (int col = 0; col < 2 * n_depth + dim; col++)
        superA[0][col] = superA[n_depth + dim][col] = 0;
    for (int row = 1; row < n_depth + dim; row++)
        for (int col = 1; col < n_depth + dim; col++)
            superA[row][col] = A[row][col];

    // int row = 1;
    // int col = 1;
    int nd = 1;

    // int r = row - nd;
    // int c = col - nd;
    int l = 2 * nd + 1;

    // for (int i = 0; i < l; i++)
    // {
    //     for (int j = 0; j < l; j++)
    //     {
    //         chain[i * l + j] = A[r + i][c + j];
    //     }
    // }

    for (int i = 0; i <= dim; i++)
    {
        MPI_Scatter(&superA[i], dim, MPI_INT, &B[i], dim, MPI_INT, 0, MPI_COMM_WORLD);
    }

    /// Up to here, finish printing B out and checking if we can get n_depth of 0s surrounding the original arrow

    printf("Hello, i'm process: %d\n", me);

    printf("Row 0: %d %d %d\n", B[-1][0], B[-1][0], B[-1][0]);
    printf("Row 1: %d %d %d\n", B[0][0], B[0][1], B[0][2]);
    printf("Row 2: %d %d %d\n", B[1][0], B[1][1], B[1][2]);
    printf("Row 3: %d %d %d\n", B[2][0], B[2][1], B[2][2]);
    printf("Row 4: %d %d %d\n", B[3][0], B[3][1], B[3][2]);
    // for (int i = 0; i <= dim; i++)
    // {
    //     for (int j = 0; j <= dim; j++)
    //     {
    //         printf(" %d %d %d\n", B[i][j - 1], B[i][j], B[i][j + 1]);
    //     }
    //     printf("\n");
    // }

    MPI_Finalize();

    return 0;
}