#include "new_mpi.h"
#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;
    int data;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) printf("Нужно хотя бы 2 процесса\n");
        MPI_Finalize();
        return 1;
    }

    // Инициализация: каждый процесс хранит свой номер
    data = rank;
    printf("Процесс %d: начальное значение = %d\n", rank, data);

    // Определяем соседей в кольце
    int left = (rank - 1 + size) % size;   // от кого получаем
    int right = (rank + 1) % size;         // кому отправляем

    // Обмен с заменой: отправляем data правому, получаем от левого в ту же переменную
    MPI_Sendrecv_replace(&data, 1, MPI_INT,
                         right, 0,        // send to right, tag 0
                         left, 0,         // recv from left, tag 0
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Процесс %d: получено значение = %d\n", rank, data);

    MPI_Finalize();
    return 0;
}