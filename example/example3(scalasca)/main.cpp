#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Функция с неравномерной нагрузкой
void uneven_work(int rank, int size) {
    if (rank == 0) {
        // Процесс 0 делает много работы
        printf("Process %d: Doing heavy work...\n", rank);
        double result = 0.0;
        for (long i = 0; i < 100000000; i++) {
            result += i * 0.001;
        }
        printf("Process %d: Heavy work completed: %f\n", rank, result);
    } else {
        // Остальные процессы делают мало работы
        printf("Process %d: Doing light work...\n", rank);
        usleep(100000); // 100ms
    }
}

// Функция с неэффективной коммуникацией
void inefficient_communication(int rank, int size) {
    int data[1000];
    
    // Процесс 0 отправляет много мелких сообщений
    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            for (int j = 0; j < 100; j++) {
                MPI_Send(data, 10, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        // Остальные процессы принимают сообщения
        MPI_Status status;
        for (int j = 0; j < 100; j++) {
            MPI_Recv(data, 10, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    printf("Process %d started\n", rank);
    
    // Синхронизация в начале
    MPI_Barrier(MPI_COMM_WORLD);
    
    // УЗКОЕ МЕСТО 1: Неравномерная нагрузка
    uneven_work(rank, size);
    
    // Синхронизация
    MPI_Barrier(MPI_COMM_WORLD);
    
    // УЗКОЕ МЕСТО 2: Неэффективная коммуникация
    inefficient_communication(rank, size);
    
    // УЗКОЕ МЕСТО 3: Все ждут один процесс
    if (rank == 0) {
        printf("Root process preparing data...\n");
        sleep(2); // Долгая подготовка данных
    }
    MPI_Bcast(&rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    printf("Process %d finished\n", rank);
    MPI_Finalize();
    return 0;
}