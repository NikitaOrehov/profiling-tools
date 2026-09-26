#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define N 1000000000

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int* buf = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) buf[i] = rank;

    if (rank == 0) {
        MPI_Request req[3];
        printf("P0: Isend ×3 (large)\n");
        MPI_Isend(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD, &req[0]);
        MPI_Isend(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD, &req[1]);
        MPI_Isend(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD, &req[2]);
        printf("P0: Waitall\n");
        MPI_Waitall(3, req, MPI_STATUSES_IGNORE);   // зависнет на третьем
        printf("P0: завершено\n");
    } else if (rank == 1) {
        MPI_Recv(buf, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(buf, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("P1: больше не принимаю\n");
    }

    MPI_Finalize();
    free(buf);
    return 0;
}