#include "new_mpi.h"
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int val = rank;

    if (rank == 0) {
    MPI_Barrier(MPI_COMM_WORLD);   // все прошли — значит P1 уже сделал Irecv

    printf("P0: Rsend 1\n");
    MPI_Rsend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    printf("P0: Rsend 2\n");
    MPI_Rsend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    printf("P0: Rsend 3 (Recv не posted)\n");
    MPI_Rsend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);  // теперь UB, потому что P1 сделал только 2 Irecv
    printf("P0: завершено\n");
} else if (rank == 1) {
    MPI_Request req[2];
    MPI_Recv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Barrier(MPI_COMM_WORLD);   // оба процесса встречаются здесь

    printf("P1: больше не принимаю\n");
}

    MPI_Finalize();
    return 0;
}