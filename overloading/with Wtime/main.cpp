#include "new_mpi.h"
#include <iostream>
#include <random>



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
        MPI_Send(a, size_a, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
    if (rank == 1){
        MPI_Recv(a, size_a, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 0; i < size_a; i++){
            std::cout << a[i] << " ";
        }
        std::cout << "\n";
    }
    MPI_Bcast(&size_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int result;
    MPI_Reduce(&size_a, &result, 1, MPI_INT, MPI_SUM, 0,MPI_COMM_WORLD);
    if (rank == 0) {
        std::cout << result << "\n";
    }
    MPI_Finalize();
    return 0;
}
