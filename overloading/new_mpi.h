#pragma once
#include <mpi.h>
#include <iostream>
#include "trace_collector.h"


/* Инициализация и завершение */
int MPI_Init(int *argc, char ***argv){
    item i {.name = "MPI_Init"};
    i.start = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - collector._start)).count();

    int result = PMPI_Init(argc, argv);
    
    i.end = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - collector._start)).count();
    
    collector.push_back(i);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    collector.set_process(rank);

    return result;
}

int MPI_Finalize(void){
    item i {.name = "MPI_Finalize"};
    i.start = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - collector._start)).count();
    
    int result = PMPI_Finalize();
    
    i.end = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - collector._start)).count();
    
    collector.push_back(i);

    return result;
}

/* Точечные коммуникации */
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    item i {.name = "MPI_Send"};
    i.start = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - collector._start)).count();
    
    int result = PMPI_Send(buf, count, datatype, dest, tag, comm);
    
    i.end = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - collector._start)).count();
    
    collector.push_back(i);
    
    return result;
}
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status){
    item i {.name = "MPI_Recv"};
    i.start = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - collector._start)).count();
    
    int result = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
    
    i.end = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - collector._start)).count();
    
    collector.push_back(i);
    
    return result;
}

// int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
// int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);

// /* Коллективные операции */
// int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
// int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
// int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
// int MPI_Barrier(MPI_Comm comm);
// int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
// int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);

// /* Информация о коммуникаторе */
// int MPI_Comm_size(MPI_Comm comm, int *size);
// int MPI_Comm_rank(MPI_Comm comm, int *rank);

// /* Время */
// double MPI_Wtime(void);
// double MPI_Wtick(void);