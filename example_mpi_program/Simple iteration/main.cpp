#include "new_mpi.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 200
#define FIXED_ITER 1214

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = N;
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int base = n / size;
    int rem  = n % size;

    int* counts = (int*)malloc(size * sizeof(int));
    int* displs = (int*)malloc(size * sizeof(int));
    int offset = 0;
    for (int i = 0; i < size; i++) {
        counts[i] = base + (i < rem ? 1 : 0);
        displs[i] = offset;
        offset += counts[i];
    }

    int local_n = counts[rank];

    double* A_full = NULL;
    double* b_full = NULL;

    if (rank == 0) {
        A_full = (double*)malloc(n * n * sizeof(double));
        b_full = (double*)malloc(n * sizeof(double));
        srand(11);
        for (int i = 0; i < n; i++) {
            double diag = 0;
            for (int j = 0; j < n; j++) {
                A_full[i*n + j] = (i == j) ? 0 : (double)rand() / RAND_MAX;
                diag += fabs(A_full[i*n + j]);
            }
            A_full[i*n + i] = diag + 1.0;
            b_full[i] = (double)rand() / RAND_MAX * 100.0;
        }
    }

    /* Локальный блок A: local_n строк по n элементов */
    double* A_local = (double*)malloc(local_n * n * sizeof(double));
    double* b_local = (double*)malloc(local_n * sizeof(double));

    int* counts_A = (int*)malloc(size * sizeof(int));
    int* displs_A = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        counts_A[i] = counts[i] * n;
        displs_A[i] = displs[i] * n;
    }

    /* На не-корневых sendbuf = NULL допустим по стандарту, MS-MPI строг —
       поэтому передаём MPI_IN_PLACE не получится, а вот «dummy» буфер
       на корне дадим. */
    MPI_Scatterv(A_full, counts_A, displs_A, MPI_DOUBLE,
                 A_local, local_n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b_full, counts, displs, MPI_DOUBLE,
                 b_local, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double* x_global = (double*)calloc(n, sizeof(double));
    double* x_new_local = (double*)malloc(local_n * sizeof(double));

    for (int iter = 0; iter < FIXED_ITER; iter++) {
        double local_diff = 0;
        for (int i = 0; i < local_n; i++) {
            int gi = displs[rank] + i;
            double sum = b_local[i];
            for (int j = 0; j < n; j++) {
                if (j != gi) sum -= A_local[i*n + j] * x_global[j];
            }
            double xi_new = sum / A_local[i*n + gi];
            local_diff += fabs(xi_new - x_global[gi]);
            x_new_local[i] = xi_new;
        }

        MPI_Allgatherv(x_new_local, local_n, MPI_DOUBLE,
                       x_global, counts, displs, MPI_DOUBLE,
                       MPI_COMM_WORLD);

        double global_diff = 0;
        MPI_Allreduce(&local_diff, &global_diff, 1, MPI_DOUBLE,
                      MPI_SUM, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        printf("Итераций: %d\n", FIXED_ITER);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    free(A_full); free(b_full);
    free(A_local); free(b_local);
    free(x_global); free(x_new_local);
    free(counts); free(displs);
    free(counts_A); free(displs_A);

    MPI_Finalize();
    return 0;
} // mpiexec -n 5 ./main