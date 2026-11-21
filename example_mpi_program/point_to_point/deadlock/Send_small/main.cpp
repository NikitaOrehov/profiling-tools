#include "new_mpi.h"
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int val = rank;

    if (rank == 0) {
        printf("P0: Send 1\n");
        MPI_Send(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: Send 2\n");
        MPI_Send(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: Send 3 (нет пары)\n");
        MPI_Send(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: все Send завершены\n");
    } else if (rank == 1) {
        printf("P1: Recv 1\n");
        MPI_Recv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("P1: Recv 2\n");
        MPI_Recv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("P1: больше не принимаю\n");
    }

    MPI_Finalize();
    return 0;
}