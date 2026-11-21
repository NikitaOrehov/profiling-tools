#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define EXCHANGES  100
#define MSG_SHORT  1          /* 4 байта */
#define MSG_MEDIUM 10000      /* 40 КБ */
#define MSG_LONG   1000000    /* 4 МБ */

static void run_benchmark(int rank, int n, MPI_Datatype dtype) {
    int* buf = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) buf[i] = rank;

    int partner = 1 - rank;

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Request request;

    for (int it = 0; it < EXCHANGES; it++) {
        if (rank == 0) {
            MPI_Isend(buf, n, dtype, partner, 0, MPI_COMM_WORLD, &request);
        } else {
            MPI_Irecv(buf, n, dtype, partner, 0, MPI_COMM_WORLD, &request);
        }
    }

    free(buf);
    return ;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) printf("Требуется ровно 2 процесса\n");
        MPI_Finalize();
        return 1;
    }

    run_benchmark(rank, MSG_MEDIUM,  MPI_INT);

    MPI_Finalize();
    return 0;
}