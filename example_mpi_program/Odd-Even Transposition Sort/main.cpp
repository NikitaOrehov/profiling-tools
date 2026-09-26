#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define N 2000

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = 0;
    double* data = NULL;

    if (rank == 0) {
        n = N;
        data = (double*)malloc(N * sizeof(double));
        srand(42);
        for (int i = 0; i < N; i++) data[i] = (double)rand() / RAND_MAX * 1000.0;
    }
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
    double* local = (double*)malloc((local_n + 1) * sizeof(double));

    MPI_Scatterv(data, counts, displs, MPI_DOUBLE,
                 local, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < local_n - 1; i++)
        for (int j = 0; j < local_n - 1 - i; j++)
            if (local[j] > local[j+1]) {
                double t = local[j]; local[j] = local[j+1]; local[j+1] = t;
            }

    for (int phase = 0; phase < n; phase++) {
        int parity = phase & 1;

        int start = ((displs[rank] & 1) == parity) ? 0 : 1;
        for (int i = start; i + 1 < local_n; i += 2)
            if (local[i] > local[i+1]) {
                double t = local[i]; local[i] = local[i+1]; local[i+1] = t;
            }

        if (rank + 1 < size && local_n > 0 && counts[rank+1] > 0) {
            int last_global = displs[rank] + local_n - 1;
            if ((last_global & 1) == parity) {
                double send = local[local_n-1], recv = 0;
                MPI_Sendrecv(&send, 1, MPI_DOUBLE, rank+1, phase,
                             &recv, 1, MPI_DOUBLE, rank+1, phase,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (recv < send) local[local_n-1] = recv;
            }
        }

        if (rank - 1 >= 0 && local_n > 0 && counts[rank-1] > 0) {
            int first_global = displs[rank];
            int boundary_left = first_global - 1;
            if ((boundary_left & 1) == parity) {
                double send = local[0], recv = 0;
                MPI_Sendrecv(&send, 1, MPI_DOUBLE, rank-1, phase,
                             &recv, 1, MPI_DOUBLE, rank-1, phase,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (recv > send) local[0] = recv;
            }
        }
    }

    /* ВАЖНО: буфер приёма должен быть валиден НА ВСЕХ процессах */
    double* result = (double*)malloc(n * sizeof(double));
    MPI_Allgatherv(local, local_n, MPI_DOUBLE,
                   result, counts, displs, MPI_DOUBLE, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Проверка: first=%.3f last=%.3f\n", result[0], result[N-1]);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    free(data); free(local); free(result); free(counts); free(displs);

    MPI_Finalize();
    return 0;
}
//mpiexec -n 6 ./main