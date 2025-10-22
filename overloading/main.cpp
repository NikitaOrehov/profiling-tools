
#include <mpi.h>
#include <iostream>
#include <dlfcn.h>
#include <random>

static int (*real_PMPI_Init)(int* argc, char*** argv) = nullptr;

int MPI_Init(int* argc, char*** argv) {
    std::cout << "🎯 Перехвачен вызов MPI_Init!\n";

    if (real_PMPI_Init == nullptr) {
        real_PMPI_Init = (int (*)(int*, char***))dlsym(RTLD_NEXT, "PMPI_Init");
        
        if (real_PMPI_Init == nullptr) {
            std::cerr << "❌ Ошибка: Не найдена PMPI_Init\n";
            return -1;
        }
    }

    std::cout << "🎯 1\n";
    
    return real_PMPI_Init(argc, argv);
}

int main(int* argc, char*** argv){
    MPI_Init(argc, argv);
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size_a = 5;
    int* a = new int[size_a];
    if (rank == 0){
        for (int i = 0; i < size_a; i++){
            a[i] = rand() % size;
        }
    }
    MPI_Bcast(a, size_a, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

// Закрываем extern "C" блок