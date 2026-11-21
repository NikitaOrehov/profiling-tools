#include "new_mpi.h"
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int val = rank;

    if (rank == 0) {
        printf("P0: Ssend 1\n");
        MPI_Ssend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: Ssend 2\n");
        MPI_Ssend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: Ssend 3 (без пары)\n");
        MPI_Ssend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);  // зависнет
        printf("P0: завершено\n");
    } else if (rank == 1) {
        MPI_Recv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("P1: больше не принимаю\n");
    }

    MPI_Finalize();
    return 0;
}