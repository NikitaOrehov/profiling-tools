#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <fstream>
#include <memory>
#include "trace_collector.h"

static std::unique_ptr<TraceCollector> global_collector = std::make_unique<TraceCollector>();;

#define TRACE_MPI_POINT_TO_POINT(func_name, dest, ...) \
    do { \
        if (!global_collector) break; \
        TraceItem item; \
        item.name = #func_name; \
        if (dest != MPI_PROC_NULL && dest != -1) item.dests.push_back(dest); \
        item.start = global_collector->get_relative_time_us(); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        item.end = global_collector->get_relative_time_us(); \
        global_collector->push_back(item); \
        return result; \
    } while(0)

// Макрос для коллективных операций (несколько dests)
#define TRACE_MPI_COLLECTIVE(func_name, dests_vector, ...) \
    do { \
        if (!global_collector) break; \
        TraceItem item; \
        item.name = #func_name; \
        item.dests = dests_vector; \
        item.start = global_collector->get_relative_time_us(); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        item.end = global_collector->get_relative_time_us(); \
        global_collector->push_back(item); \
        return result; \
    } while(0)

// Макрос для операций без dests
#define TRACE_MPI_SIMPLE(func_name, ...) \
    do { \
        if (!global_collector) break; \
        TraceItem item; \
        item.name = #func_name; \
        item.start = global_collector->get_relative_time_us(); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        item.end = global_collector->get_relative_time_us(); \
        global_collector->push_back(item); \
        return result; \
    } while(0)



int MPI_Init(int *argc, char ***argv) {
    //global_collector = std::make_unique<TraceCollector>();

    auto chrono_start = std::chrono::steady_clock::now();
    int result = PMPI_Init(argc, argv);
    auto chrono_end = std::chrono::steady_clock::now();
    auto init_duration = chrono_end - chrono_start;
    
    double init_duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
        init_duration).count();
    
    TraceItem item;
    item.name = "MPI_Init";
    item.start = 0;
    item.end = init_duration_us;
    global_collector->push_back(item);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    global_collector->set_process(rank);
    global_collector->CreateFolder();

    return result;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    TRACE_MPI_POINT_TO_POINT(Send, dest, buf, count, datatype, dest, tag, comm);
}

int MPI_Isend(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request) {
    TRACE_MPI_POINT_TO_POINT(Isend, dest, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
    TRACE_MPI_SIMPLE(Recv, buf, count, datatype, source, tag, comm, status);
}

int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request) {
    TRACE_MPI_SIMPLE(Irecv, buf, count, datatype, source, tag, comm, request);
}


int MPI_Gather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
               void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    
    if (rank == root) {
        // Root получает от всех процессов
        int size;
        MPI_Comm_size(comm, &size);
        std::vector<int> sources;
        for (int i = 0; i < size; i++) {
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Gather, sources, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
    } else {
        // Не-root процессы отправляют root'у
        TRACE_MPI_POINT_TO_POINT(Gather, root, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
    }
}


int MPI_Scatter(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    
    if (rank == root) {
        // Root отправляет всем процессам
        int size;
        MPI_Comm_size(comm, &size);
        std::vector<int> dests;
        for (int i = 0; i < size; i++) {
            if (i != root) dests.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Scatter, dests, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
    } else {
        // Не-root процессы получают от root'а (без dests)
        TRACE_MPI_SIMPLE(Scatter, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
    }
}


int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    
    if (rank == root) {
        // Root отправляет всем процессам
        int size;
        MPI_Comm_size(comm, &size);
        std::vector<int> dests;
        for (int i = 0; i < size; i++) {
            if (i != root) dests.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Bcast, dests, buffer, count, datatype, root, comm);
    } else {
        // Не-root процессы получают (без dests)
        TRACE_MPI_SIMPLE(Bcast, buffer, count, datatype, root, comm);
    }
}


int MPI_Alltoall(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                 void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
    int size;
    MPI_Comm_size(comm, &size);
    std::vector<int> dests(size);
    for (int i = 0; i < size; i++) {
        dests[i] = i;
    }
    TRACE_MPI_COLLECTIVE(Alltoall, dests, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}


int MPI_Reduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    
    if (rank == root) {
        // Root получает от всех процессов
        int size;
        MPI_Comm_size(comm, &size);
        std::vector<int> sources;
        for (int i = 0; i < size; i++) {
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Reduce, sources, sendbuf, recvbuf, count, datatype, op, root, comm);
    } else {
        // Не-root процессы отправляют root'у
        TRACE_MPI_POINT_TO_POINT(Reduce, root, sendbuf, recvbuf, count, datatype, op, root, comm);
    }
}


int MPI_Barrier(MPI_Comm comm) {
    TRACE_MPI_SIMPLE(Barrier, comm);
}

int MPI_Finalize(void) {
    TRACE_MPI_SIMPLE(Finalize);
}