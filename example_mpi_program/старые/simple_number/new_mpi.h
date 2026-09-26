#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <fstream>
#include <memory>
#include "trace_collector.h"

static std::unique_ptr<TraceCollector> global_collector = std::make_unique<TraceCollector>();

void firstInit(){
    static bool initialization = false;
    if (!initialization && global_collector){
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        global_collector->set_process(rank);
        global_collector->CreateFolder();
        initialization = true;
    }
}

// Макрос для коллективных операций (несколько dests)
#define TRACE_MPI_COLLECTIVE(func_name, dests_vector, ...) \
    do { \
        if (!global_collector) break; \
        firstInit(); \
        TraceItem item; \
        item.name = #func_name; \
        item.dests = dests_vector; \
        item.start = global_collector->get_relative_time_us(); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        item.end = global_collector->get_relative_time_us(); \
        global_collector->push_back(item); \
        return result; \
    } while(0); \
    return PMPI_##func_name(__VA_ARGS__)

// Макрос для операций без dests
#define TRACE_MPI_SIMPLE(func_name, ...) \
    do { \
        if (!global_collector) break; \
        firstInit(); \
        TraceItem item; \
        item.name = #func_name; \
        item.start = global_collector->get_relative_time_us(); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        item.end = global_collector->get_relative_time_us(); \
        global_collector->push_back(item); \
        return result; \
    } while(0); \
    return PMPI_##func_name(__VA_ARGS__)


int MPI_Init(int *argc, char ***argv) {
    std::cout << "Init\n";
    auto chrono_start = std::chrono::steady_clock::now();
    int result = PMPI_Init(argc, argv);
    auto chrono_end = std::chrono::steady_clock::now();
    auto init_duration = chrono_end - chrono_start;
    
    double init_duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
        init_duration).count();

    firstInit();
    
    TraceItem item;
    item.name = "MPI_Init";
    item.start = 0;
    item.end = init_duration_us;
    global_collector->push_back(item);

    return result;
}


//===================================POINT-TO-POINT===================================================================
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    TRACE_MPI_COLLECTIVE(Send, {dest}, buf, count, datatype, dest, tag, comm);
}

int MPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    TRACE_MPI_COLLECTIVE(Ssend, {dest}, buf, count, datatype, dest, tag, comm);
}

int MPI_Issend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request){
    TRACE_MPI_COLLECTIVE(Issend, {dest}, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Rsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    TRACE_MPI_COLLECTIVE(Rsend, {dest}, buf, count, datatype, dest, tag, comm);
}
int MPI_Irsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request){
    TRACE_MPI_COLLECTIVE(Irsend, {dest}, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Isend(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request) {
    TRACE_MPI_COLLECTIVE(Isend, {dest}, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Bsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    TRACE_MPI_COLLECTIVE(Bsend, {dest}, buf, count, datatype, dest, tag, comm);
}

int MPI_Ibsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request){
    TRACE_MPI_COLLECTIVE(Ibsend, {dest}, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
    std::vector<int> sources = {-1, source};
    TRACE_MPI_COLLECTIVE(Recv, sources, buf, count, datatype, source, tag, comm, status);
}

int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request) {
    std::vector<int> sources = {-1, source};
    TRACE_MPI_COLLECTIVE(Irecv, sources, buf, count, datatype, source, tag, comm, request);
}

int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag,
                 MPI_Comm comm, MPI_Status *status) {
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &rank);
    std::vector<int> vec_source;
    vec_source.push_back(dest);
    TRACE_MPI_COLLECTIVE(Sendrecv, vec_source, sendbuf, sendcount, sendtype, dest, sendtag, 
        recvbuf, recvcount, recvtype, source, recvtag, comm, status);
}

int MPI_Sendrecv_replace(void *buf, int count, MPI_Datatype datatype, int dest, int sendtag,
                         int source, int recvtag, MPI_Comm comm, MPI_Status *status) {
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &rank);
    std::vector<int> vec_source;
    vec_source.push_back(dest);
    TRACE_MPI_COLLECTIVE(Sendrecv_replace, vec_source, buf, count, datatype, dest, sendtag, 
            source, recvtag, comm, status);
}




//===================================COLLECTIVE===================================================================

int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    std::vector<int> dests;
    if (rank == root) {
        for (int i = 0; i < size; i++) {
            if (i != root) dests.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Bcast, dests, buffer, count, datatype, root, comm);
    } else {
        dests.push_back(-1);
        dests.push_back(root);
        TRACE_MPI_COLLECTIVE(Bcast, dests, buffer, count, datatype, root, comm);
    }
}


int MPI_Reduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    
    if (rank == root) {
        int size;
        MPI_Comm_size(comm, &size);
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++) {
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Reduce, sources, sendbuf, recvbuf, count, datatype, op, root, comm);
    } else {
        TRACE_MPI_COLLECTIVE(Reduce, {root}, sendbuf, recvbuf, count, datatype, op, root, comm);
    }
}

int MPI_Gather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
              void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == root){
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Gather, sources, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm);
    }
    else{
        TRACE_MPI_COLLECTIVE(Gather, {root}, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm);
    }
}

int MPI_Scatter(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
               void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == root){
        std::vector<int> sources;
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Scatter, sources, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm);
    }
    else{
        std::vector<int> dests = {-1, root};
        TRACE_MPI_COLLECTIVE(Scatter, dests, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm);
    }
}

int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == root){
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Gatherv, sources, sendbuf, sendcount, sendtype,
                recvbuf, recvcounts, displs, recvtype, root, comm);
    }
    else{
        TRACE_MPI_COLLECTIVE(Gatherv, {root}, sendbuf, sendcount, sendtype,
               recvbuf, recvcounts, displs, recvtype, root, comm);
    }
}

int MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype, int root, MPI_Comm comm){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == root){
        std::vector<int> sources;
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Scatterv, sources, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount,
                recvtype, root, comm);
    }
    else{
        std::vector<int> dests = {-1, root};
        TRACE_MPI_COLLECTIVE(Scatterv, dests, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount,
                recvtype, root, comm);
    }
}


//===================================COLLECTIVE-I===================================================================

int MPI_Ibcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm, MPI_Request* request) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    std::vector<int> dests;
    if (rank == root) {
        for (int i = 0; i < size; i++) {
            if (i != root) dests.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Ibcast, dests, buffer, count, datatype, root, comm, request);
    } else {
        dests.push_back(-1);
        dests.push_back(root);
        TRACE_MPI_COLLECTIVE(Ibcast, dests, buffer, count, datatype, root, comm, request);
    }
}


int MPI_Ireduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm, MPI_Request* request) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    
    if (rank == root) {
        int size;
        MPI_Comm_size(comm, &size);
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++) {
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Ireduce, sources, sendbuf, recvbuf, count, datatype, op, root, comm, request);
    } else {
        TRACE_MPI_COLLECTIVE(Ireduce, {root}, sendbuf, recvbuf, count, datatype, op, root, comm, request);
    }
}

int MPI_Igather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
              void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == root){
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Igather, sources, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm, request);
    }
    else{
        TRACE_MPI_COLLECTIVE(Igather, {root}, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm, request);
    }
}

int MPI_Iscatter(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
               void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == root){
        std::vector<int> sources;
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Iscatter, sources, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm, request);
    }
    else{
        std::vector<int> dests = {-1, root};
        TRACE_MPI_COLLECTIVE(Iscatter, dests, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm, request);
    }
}

int MPI_Igatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == root){
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Igatherv, sources, sendbuf, sendcount, sendtype,
                recvbuf, recvcounts, displs, recvtype, root, comm, request);
    }
    else{
        TRACE_MPI_COLLECTIVE(Igatherv, {root}, sendbuf, sendcount, sendtype,
               recvbuf, recvcounts, displs, recvtype, root, comm, request);
    }
}

int MPI_Iscatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == root){
        std::vector<int> sources;
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Iscatterv, sources, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount,
                recvtype, root, comm, request);
    }
    else{
        std::vector<int> dests = {-1, root};
        TRACE_MPI_COLLECTIVE(Iscatterv, dests, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount,
                recvtype, root, comm, request);
    }
}



//===================================ALL-OPERATIONS===================================================================

int MPI_Allreduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
                  MPI_Op op, MPI_Comm comm){
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(i);
    }
    TRACE_MPI_COLLECTIVE(Allreduce, dests, sendbuf, recvbuf, count, datatype, op, comm);
}

int MPI_Allgather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                  void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm){
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(i);
    }
    TRACE_MPI_COLLECTIVE(Allgather, dests, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}

int MPI_Alltoall(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                 void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm){
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(i);
    }
    TRACE_MPI_COLLECTIVE(Alltoall, dests, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}


//===================================ALL-OPERATIONS-I===================================================================

int MPI_Iallreduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
                  MPI_Op op, MPI_Comm comm, MPI_Request* request){
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(i);
    }
    TRACE_MPI_COLLECTIVE(Iallreduce, dests, sendbuf, recvbuf, count, datatype, op, comm, request);
}

int MPI_Iallgather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                  void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request){
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(i);
    }
    TRACE_MPI_COLLECTIVE(Iallgather, dests, sendbuf, sendcount, sendtype, recvbuf,
         recvcount, recvtype, comm, request);
}

int MPI_Ialltoall(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                 void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request){
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(i);
    }
    TRACE_MPI_COLLECTIVE(Ialltoall, dests, sendbuf, sendcount, sendtype, recvbuf,
         recvcount, recvtype, comm, request);
}





int MPI_Barrier(MPI_Comm comm) {
    TRACE_MPI_SIMPLE(Barrier, comm);
}

int MPI_Ibarrier(MPI_Comm comm, MPI_Request* request) {
    TRACE_MPI_SIMPLE(Ibarrier, comm, request);
}

int MPI_Finalize(void) {
    TRACE_MPI_SIMPLE(Finalize);
}


