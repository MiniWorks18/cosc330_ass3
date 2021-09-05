#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "matrix.c"

void printRow(int A[], int me, int dim) {
    for (int i = 0; i < dim; i++)
        printf(" %d", A[i]);
    printf("\n");
}

int parse_args(int argc, char *argv[], int *dim, int *n_depth, int *fd) {
    if (argc != 5 
        || ((*dim = atoi(argv[1])) <= 0)
        || ((*n_depth = atoi(argv[2])) <= 0)
        || ((fd[0] = open(argv[3], O_RDONLY)) == -1) 
        || ((fd[1] = open(argv[4], O_WRONLY | O_CREAT, 0666)) == -1)) {
        fprintf(stderr,
        "Usage: mpirun -n %s dimension neighbour_depth inputfile outputfile\n", argv[0]);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int me, size, dim, sum, n_sum, row, col, n_depth, i, j, x, n, fd[2];


    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (me == 0) {
        if (parse_args(argc, argv, &dim, &n_depth, fd) == -1) {
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
    }

    // Share information from root
    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n_depth, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int A[dim+1][dim+1], sDim = dim+2*n_depth, superA[sDim][sDim];
    int B[dim+1][dim+1];
    int Arow[n_depth*2][sDim];

    // This is the weirdest behaviour i've ever seen
    int result_row[1000000];


    int Orow[dim+1][dim+1];

    if (me == 0) {
        // Define test matrix of variable dimension size
        for (i = 1; i < dim+1; i++) {
            if (get_row(fd[0], dim, i, &A[i][1]) == -1) {
                fprintf(stderr, "Initialization of A failed\n");
                MPI_Finalize();
                exit(EXIT_FAILURE);
            }
            // for (j = 0; j < dim; j++) {
            //     A[i][j] = i+j*2;
            //     B[i][j] = 0;
            // }
        }


        // Print matrix to console
        if (me == 0) {
            printf("Before:\n");
            for (i = 1; i < dim+1; i++) {
                for (j = 1; j < dim+1; j++) 
                    printf(" %d", A[i][j]);
                printf("\n");
            }
                // printRow(A[i], me, dim);
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
        for (i = 1; i < dim+1; i++) {
            for (j = 1; j < dim+1; j++) // Up to here, imported file is weird and needs to fit into a designed matrix size
                superA[i+n_depth-1][j+n_depth-1] = A[i][j];
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

    // printf("Process %d got:\n", me);
    // for (i = 0; i < 2*n_depth+1; i++) {
    //     for (j = 0; j < sDim; j++) 
    //         printf(" %d", Arow[i][j]);
    //     printf("\n");
    // }


    x = n_depth;
    int index = 0;
    // For each cell in our designated row to calculate
    for (int y = n_depth, j = 0; y < n_depth+dim; y++) {
        j++;
        index++;
        sum = 0;
        // For each layer of neighbours
        for (n = 1; n <= n_depth; n++) {
            n_sum = 0;
            // Sum top and bottom neighbours
            for (i = y-n; i <= y+n; i++) {
                // if (me == 0)
                    // printf("%d += %d + %d\n", n_sum, Arow[x-n][i],  Arow[x+n][i]);
                //     printf("Should not changeA:\n");
                // for (int b = 0; b < 2*n_depth+1; b++) {
                //     for (int c = 0; c < sDim; c++) 
                //         printf(" %d", Arow[b][c]);
                //     printf("\n");
                // }
                n_sum += Arow[x-n][i];
                n_sum += Arow[x+n][i];
                // printf("Should not changeB:\n");
                // for (int b = 0; b < 2*n_depth+1; b++) {
                //     for (int c = 0; c < sDim; c++) 
                //         printf(" %d", Arow[b][c]);
                //     printf("\n");
                // }
            }
            // Sum left and right neighbours
            for (i = x-n+1; i <= x+n-1; i++) {
                // if (me == 0)
                    // printf("%d += %d + %d \n", n_sum, Arow[i][y-n], Arow[i][y+n]);
                n_sum += Arow[i][y-n];
                n_sum += Arow[i][y+n];
            }


            // if (me == 0) {
                // printf("Should not change:\n");
                // for (i = 0; i < 2*n_depth+1; i++) {
                //     for (j = 0; j < sDim; j++) 
                //         printf(" %d", Arow[i][j]);
                //     printf("\n");
                // }
            // }
            // Add weighted sum to total sum
            // if (me == 0)
            // printf("%d += %d/%d\n", sum, n_sum, n);
            sum += n_sum/n;
        }
        // Print total sum
        // if (me == 0)
        // printf("Arow[%d][%d] (%d) sum: %d\n", x, y, Arow[x][y], sum);
        // printf("Brow[%d] = %d\n", index, sum);
        result_row[index] = sum;
    }
    
    if (MPI_Gather(&result_row[0], dim+1, MPI_INT,
                    &B[1][0], dim+1, MPI_INT,
                    0, MPI_COMM_WORLD) != MPI_SUCCESS) {
                        fprintf(stderr, "Gathering of Product failed\n");
                        MPI_Finalize();
                        exit(EXIT_FAILURE);
                    }

   
    if (me == 0) {
        // for (j = 0; j < dim+1; j++) {
        //     for (i = 0; i < dim+1; i++) {
        //         Orow[i][j+1] = B[i][j];
        //     }
        // }
        printf("B After:\n");
        for (i = 0; i < dim+1; i++) {
            for (j = 0; j < dim+1; j++) 
                printf(" %d", B[i][j]);
            printf("\n");
        }
        // printf("O After:\n");
        // for (i = 0; i < dim+1; i++) {
        //     for (j = 0; j < dim+1; j++) 
        //         printf(" %d", Orow[i][j]);
        //     printf("\n");
        // }
        for (i = 1; i < dim+1; i++) {
            if (set_row(fd[1], dim, i, &B[i][1]) == -1) {
                fprintf(stderr, "Writing of matrix outfile failed\n");
                goto fail;
            }
        }
        printf("\n\n");
    }


    MPI_Finalize();
    exit(EXIT_SUCCESS);
    // return 0;

    fail:
        fprintf(stderr, "%s aborted\n", argv[0]);
        MPI_Finalize();
        exit(EXIT_FAILURE);
}