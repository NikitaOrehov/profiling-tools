#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define N 1000000

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int* buf = new int[N];
    for (int i = 0; i < N; i++) buf[i] = rank;

    if (rank == 0) {
        printf("P0: Send 1 (large)\n");
        MPI_Send(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: Send 2 (large)\n");
        MPI_Send(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: Send 3 (large, без пары)\n");
        MPI_Send(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD);   // здесь зависнет
        printf("P0: завершено\n");
    } else if (rank == 1) {
        printf("P1: Recv 1\n");
        MPI_Recv(buf, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("P1: Recv 2\n");
        MPI_Recv(buf, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("P1: больше не принимаю\n");
    }

    MPI_Finalize();
    free(buf);
    return 0;
}