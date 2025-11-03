#pragma once
#include <mpi.h>
#include <iostream>
#include "trace_collector.h"


/* Инициализация и завершение */
int MPI_Init(int *argc, char ***argv){

    auto chrono_start = std::chrono::steady_clock::now();
    int result = PMPI_Init(argc, argv);
    auto chrono_end = std::chrono::steady_clock::now();
    auto init_duration = chrono_end - chrono_start;
    
    // 2. Получаем MPI время после инициализации
    double mpi_time_after_init = MPI_Wtime();
    
    // 3. Вычисляем виртуальное время начала
    double init_duration_seconds = std::chrono::duration<double>(init_duration).count();
    double virtual_start_time = mpi_time_after_init - init_duration_seconds;

    collector.set_start(virtual_start_time);

    Item i;
    i.name = "MPI_Init";
    i.start = 0;
    i.end = collector.get_relative_time();
    
    collector.push_back(i);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    collector.set_process(rank);

    return result;
}

int MPI_Finalize(void){
    Item i;
    i.name = "MPI_Finalize";
    
    auto chrono_start = std::chrono::steady_clock::now();
    double mpi_start_time = collector.get_relative_time();
    i.start = mpi_start_time;

    int result = PMPI_Finalize();
    
    auto chrono_end = std::chrono::steady_clock::now();
    auto finalize_duration = chrono_end - chrono_start;
    double duration_seconds = std::chrono::duration<double>(finalize_duration).count();
    
    i.end = mpi_start_time + duration_seconds;
    
    collector.push_back(i);

    return result;
}

/* Точечные коммуникации */
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    Item i;
    i.name = "MPI_Send";
    i.start = collector.get_relative_time();

    int result = PMPI_Send(buf, count, datatype, dest, tag, comm);
    
    i.end = collector.get_relative_time();
    
    collector.push_back(i);

    return result;
}
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status){
    Item i;
    i.name = "MPI_Recv";
    i.start = collector.get_relative_time();

    int result = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
    
    i.end = collector.get_relative_time();
    
    collector.push_back(i);

    return result;
}

int MPI_Isend(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request) {
    Item i;
    i.name = "MPI_Isend";
    i.start = collector.get_relative_time();

    int result = PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
    
    i.end = collector.get_relative_time();
    
    collector.push_back(i);

    return result;
}

int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request) {
    Item i;
    i.name = "MPI_Irecv";
    i.start = collector.get_relative_time();

    int result = PMPI_Irecv(buf, count, datatype, source, tag, comm, request);
    
    i.end = collector.get_relative_time();
    
    collector.push_back(i);

    return result;
}

// /* Коллективные операции */
int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    Item i;
    i.name = "MPI_Bcast";
    i.start = collector.get_relative_time();

    int result = PMPI_Bcast(buffer, count, datatype, root, comm);
    
    i.end = collector.get_relative_time();
    
    collector.push_back(i);

    return result;
 }


//  int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm){

//  }
 //int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
 //int MPI_Barrier(MPI_Comm comm);
 //int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
 //int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);

// /* Информация о коммуникаторе */
// int MPI_Comm_size(MPI_Comm comm, int *size);
// int MPI_Comm_rank(MPI_Comm comm, int *rank);

// /* Время */
// double MPI_Wtime(void);
// double MPI_Wtick(void);