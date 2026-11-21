#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define N 1000000        /* 1 000 000 int = 4 МБ */
#define BUF_SIZE 1024    /* буфер 1 КБ — заведомо мало */

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) printf("Нужно 2 процесса\n");
        MPI_Finalize();
        return 1;
    }

    int* buf = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) buf[i] = rank;

    /* Пользовательский буфер для Bsend — намеренно маленький */
    char* attach_buf = (char*)malloc(BUF_SIZE);
    MPI_Buffer_attach(attach_buf, BUF_SIZE);

    if (rank == 0) {
        printf("P0: Bsend 1 (сообщение %d int = %d байт, буфер %d байт)\n",
               N, N * (int)sizeof(int), BUF_SIZE);
        fflush(stdout);

        int err = MPI_Bsend(buf, N, MPI_INT, 1, 0, MPI_COMM_WORLD);

        if (err != MPI_SUCCESS) {
            char errstr[MPI_MAX_ERROR_STRING];
            int len;
            MPI_Error_string(err, errstr, &len);
            printf("P0: Bsend вернул ошибку: %s\n", errstr);
        } else {
            printf("P0: Bsend завершён успешно\n");
        }
        fflush(stdout);
    } else {
        printf("P1: жду Recv\n");
        fflush(stdout);
        MPI_Recv(buf, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("P1: принял\n");
    }

    int size_buf;
    void* buf_addr;
    MPI_Buffer_detach(&buf_addr, &size_buf);
    free(attach_buf);
    free(buf);

    MPI_Finalize();
    return 0;
}