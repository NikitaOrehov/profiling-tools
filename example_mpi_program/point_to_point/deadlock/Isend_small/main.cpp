#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define N 1

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int* buf = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) buf[i] = rank;

    if (rank == 0) {
        MPI_Request req[3];

        MPI_Isend(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD, &req[0]);
        MPI_Isend(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD, &req[1]);
        MPI_Isend(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD, &req[2]);


        MPI_Waitall(3, req, MPI_STATUSES_IGNORE);

    } else if (rank == 1) {
        MPI_Request rreq[2];
        MPI_Irecv(buf, N, MPI_INT, 0, 0, MPI_COMM_WORLD, &rreq[0]);
        MPI_Irecv(buf, N, MPI_INT, 0, 0, MPI_COMM_WORLD, &rreq[1]);
        MPI_Waitall(2, rreq, MPI_STATUSES_IGNORE);
    }

    MPI_Finalize();
    free(buf);
    return 0;
}