#include <mpi.h>
#include <iostream>
#include <vector>
#include <unistd.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int DATA_SIZE = 50000;
    const int TOTAL_ITERATIONS = 5;

    std::vector<double> data(DATA_SIZE, rank * 100.0);
    MPI_Status status;
    int next = (rank + 1) % size;
    int prev = (rank - 1 + size) % size;

    // Разное время сна для каждого процесса
    int sleep_time = rank + 1; // 1, 2, 3, 4 секунды

    MPI_Barrier(MPI_COMM_WORLD);

    for (int iter = 0; iter < TOTAL_ITERATIONS; iter++) {
        if (rank == 0) {
            MPI_Send(data.data(), DATA_SIZE, MPI_DOUBLE, next, 0, MPI_COMM_WORLD);
            MPI_Recv(data.data(), DATA_SIZE, MPI_DOUBLE, prev, 0, MPI_COMM_WORLD, &status);
        } else {
            MPI_Recv(data.data(), DATA_SIZE, MPI_DOUBLE, prev, 0, MPI_COMM_WORLD, &status);
            MPI_Send(data.data(), DATA_SIZE, MPI_DOUBLE, next, 0, MPI_COMM_WORLD);
        }

        // Спим после завершения коммуникации
        sleep(sleep_time);
    }

    MPI_Finalize();
    return 0;
}