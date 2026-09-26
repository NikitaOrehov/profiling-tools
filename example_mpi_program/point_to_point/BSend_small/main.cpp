#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int buf_size = 1024 * 1024;
    char* attach_buf = (char*)malloc(buf_size + MPI_BSEND_OVERHEAD);
    MPI_Buffer_attach(attach_buf, buf_size);

    int val = rank;

    if (rank == 0) {
        printf("P0: Bsend 1\n");
        MPI_Bsend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: Bsend 2\n");
        MPI_Bsend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: Bsend 3 (без пары)\n");
        MPI_Bsend(&val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("P0: все Bsend завершены\n");
    } else if (rank == 1) {
        MPI_Recv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("P1: больше не принимаю\n");
    }

    int size;
    void* buf_addr;
    MPI_Buffer_detach(&buf_addr, &size);
    free(attach_buf);

    MPI_Finalize();
    return 0;
}