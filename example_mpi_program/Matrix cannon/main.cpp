#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 300

static void mul_n_sum(const double* a, const double* b, double* c, int bs) {
    for (int i = 0; i < bs; i++)
        for (int k = 0; k < bs; k++) {
            double fixed = a[i*bs + k];
            for (int j = 0; j < bs; j++)
                c[i*bs + j] += fixed * b[k*bs + j];
        }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = SIZE;
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int grid = (int)floor(sqrt((double)size));
    int block = n / grid;
    int block_elems = block * block;

    int working = grid * grid;
    MPI_Comm cann_comm = MPI_COMM_NULL;
    int color = (rank < working) ? 0 : MPI_UNDEFINED;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &cann_comm);

    if (cann_comm == MPI_COMM_NULL) {
        MPI_Finalize();
        return 0;
    }

    int cr;
    MPI_Comm_rank(cann_comm, &cr);

    /* Все процессы имеют валидные указатели — пусть даже NULL,
       MS-MPI требует валидный sendbuf только на корне. На не-корневых
       можно передать MPI_IN_PLACE или буфер произвольного размера. */
    double* a_blocks = NULL;
    double* b_blocks = NULL;

    if (cr == 0) {
        a_blocks = (double*)malloc(working * block_elems * sizeof(double));
        b_blocks = (double*)malloc(working * block_elems * sizeof(double));

        double* A = (double*)malloc(n * n * sizeof(double));
        double* B = (double*)malloc(n * n * sizeof(double));
        srand(7);
        for (int i = 0; i < n*n; i++) {
            A[i] = (double)rand() / RAND_MAX;
            B[i] = (double)rand() / RAND_MAX;
        }

        for (int p = 0; p < working; p++) {
            int pi = p / grid, pj = p % grid;
            int off = p * block_elems;
            for (int i = 0; i < block; i++)
                for (int j = 0; j < block; j++) {
                    int gi = pi*block + i, gj = pj*block + j;
                    a_blocks[off + i*block + j] = A[gi*n + gj];
                    b_blocks[off + i*block + j] = B[gi*n + gj];
                }
        }
        free(A); free(B);
    }

    double *a = (double*)malloc(block_elems * sizeof(double));
    double *b = (double*)malloc(block_elems * sizeof(double));
    double *c = (double*)calloc(block_elems, sizeof(double));

    MPI_Scatter(a_blocks, block_elems, MPI_DOUBLE, a, block_elems, MPI_DOUBLE, 0, cann_comm);
    MPI_Scatter(b_blocks, block_elems, MPI_DOUBLE, b, block_elems, MPI_DOUBLE, 0, cann_comm);

    int row = cr / grid, col = cr % grid;

    int left  = row*grid + ((col - row + grid) % grid);
    int right = row*grid + ((col + row) % grid);
    MPI_Sendrecv_replace(a, block_elems, MPI_DOUBLE, left, 0, right, 0, cann_comm, MPI_STATUS_IGNORE);

    int up   = (((row - col + grid) % grid) * grid) + col;
    int down = (((row + col) % grid) * grid) + col;
    MPI_Sendrecv_replace(b, block_elems, MPI_DOUBLE, up, 0, down, 0, cann_comm, MPI_STATUS_IGNORE);

    for (int it = 0; it < grid; it++) {
        mul_n_sum(a, b, c, block);

        if (it < grid - 1) {
            left  = row*grid + ((col - 1 + grid) % grid);
            right = row*grid + ((col + 1) % grid);
            MPI_Sendrecv_replace(a, block_elems, MPI_DOUBLE, left, 0, right, 0, cann_comm, MPI_STATUS_IGNORE);

            up   = (((row - 1 + grid) % grid) * grid) + col;
            down = (((row + 1) % grid) * grid) + col;
            MPI_Sendrecv_replace(b, block_elems, MPI_DOUBLE, up, 0, down, 0, cann_comm, MPI_STATUS_IGNORE);
        }
    }

    double* c_blocks = NULL;
    if (cr == 0) c_blocks = (double*)malloc(n * n * sizeof(double));
    MPI_Gather(c, block_elems, MPI_DOUBLE, c_blocks, block_elems, MPI_DOUBLE, 0, cann_comm);

    if (cr == 0) {
        double* C = (double*)malloc(n * n * sizeof(double));
        for (int p = 0; p < working; p++) {
            int pi = p / grid, pj = p % grid;
            int off = p * block_elems;
            for (int i = 0; i < block; i++)
                for (int j = 0; j < block; j++) {
                    int gi = pi*block + i, gj = pj*block + j;
                    C[gi*n + gj] = c_blocks[off + i*block + j];
                }
        }
        printf("C[0][0] = %.6f\n", C[0]);
        free(C); free(c_blocks);
    }

    free(a_blocks); free(b_blocks);
    free(a); free(b); free(c);

    MPI_Comm_free(&cann_comm);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}