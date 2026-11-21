#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <fstream>
#include <memory>
#include <map>
#include "trace_collector.h"


namespace{
    static std::unique_ptr<TraceCollector> global_collector = std::make_unique<TraceCollector>();
    static std::map<MPI_Comm, std::vector<int>> localToglobal;

    void firstInit(){
        static bool initialization = false;
        if (!initialization && global_collector){
            int rank, size;
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            MPI_Comm_size(MPI_COMM_WORLD, &size);

            global_collector->set_process(rank);
            global_collector->CreateFolder();

            std::vector<int> ranks(size);
            for (size_t i = 0; i < size; i++){
                ranks[i] = i;
            }
            localToglobal.emplace(std::pair(MPI_COMM_WORLD, ranks));

            initialization = true;
        }
    }

    void getGlobalranks(MPI_Comm& comm, std::vector<int>& global_ranks, MPI_Group* group = nullptr){
        MPI_Group local_group;
        if (!group){
            MPI_Comm_group(comm, &local_group);
        }
        else{
            local_group = *group;
        }
        MPI_Group global_group;
        MPI_Comm_group(MPI_COMM_WORLD, &global_group);

        int size;
        MPI_Group_size(local_group, &size);

        std::vector<int> local_ranks(size);
        for (size_t i = 0; i < size; i++){
            local_ranks[i] = i;
        }
        global_ranks.resize(size);

        MPI_Group_translate_ranks(local_group, size, local_ranks.data(), global_group, global_ranks.data());
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
        global_collector->push_back_begin(item); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        long long end = global_collector->get_relative_time_us(); \
        global_collector->push_back_end(end); \
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
        global_collector->push_back_begin(item); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        long long end = global_collector->get_relative_time_us(); \
        global_collector->push_back_end(end); \
        return result; \
    } while(0); \
    return PMPI_##func_name(__VA_ARGS__)


int MPI_Init(int *argc, char ***argv) {
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
    global_collector->push_back_begin(item);
    global_collector->push_back_end(init_duration_us);

    return result;
}


//===================================POINT-TO-POINT===================================================================
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    int dest_ = (localToglobal.at(comm))[dest];
    TRACE_MPI_COLLECTIVE(Send, {dest_}, buf, count, datatype, dest, tag, comm);
}

int MPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    int dest_ = (localToglobal.at(comm))[dest];
    TRACE_MPI_COLLECTIVE(Ssend, {dest_}, buf, count, datatype, dest, tag, comm);
}

int MPI_Issend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request){
    int dest_ = (localToglobal.at(comm))[dest];
    TRACE_MPI_COLLECTIVE(Issend, {dest_}, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Rsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    int dest_ = (localToglobal.at(comm))[dest];
    TRACE_MPI_COLLECTIVE(Rsend, {dest_}, buf, count, datatype, dest, tag, comm);
}
int MPI_Irsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request){
    int dest_ = (localToglobal.at(comm))[dest];
    TRACE_MPI_COLLECTIVE(Irsend, {dest_}, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Isend(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request) {
    int dest_ = (localToglobal.at(comm))[dest];
    TRACE_MPI_COLLECTIVE(Isend, {dest_}, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Bsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    int dest_ = (localToglobal.at(comm))[dest];
    TRACE_MPI_COLLECTIVE(Bsend, {dest_}, buf, count, datatype, dest, tag, comm);
}

int MPI_Ibsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request){
    int dest_ = (localToglobal.at(comm))[dest];
    TRACE_MPI_COLLECTIVE(Ibsend, {dest_}, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
    int source_ = (localToglobal.at(comm))[source];
    std::vector<int> sources = {-1, source_};
    TRACE_MPI_COLLECTIVE(Recv, sources, buf, count, datatype, source, tag, comm, status);
}

int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request) {
    int source_ = (localToglobal.at(comm))[source];
    std::vector<int> sources = {-1, source_};
    TRACE_MPI_COLLECTIVE(Irecv, sources, buf, count, datatype, source, tag, comm, request);
}

int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag,
                 MPI_Comm comm, MPI_Status *status) {
    std::vector<int> vec_source;
    int dest_, source_;
    if (source == dest){
        dest_ = (localToglobal.at(comm))[dest];
        vec_source = {-2, dest_};
    }
    else{
        dest_ = (localToglobal.at(comm))[dest];
        source_ = (localToglobal.at(comm))[source];
        vec_source = {dest_, -1, source_};
    }

    TRACE_MPI_COLLECTIVE(Sendrecv, vec_source, sendbuf, sendcount, sendtype, dest, sendtag, 
        recvbuf, recvcount, recvtype, source, recvtag, comm, status);
}

int MPI_Sendrecv_replace(void *buf, int count, MPI_Datatype datatype, int dest, int sendtag,
                         int source, int recvtag, MPI_Comm comm, MPI_Status *status) {
    std::vector<int> vec_source;
    int dest_, source_;
    if (source == dest){
        dest_ = (localToglobal.at(comm))[dest];
        vec_source = {-2, dest_};
    }
    else{
        dest_ = (localToglobal.at(comm))[dest];
        source_ = (localToglobal.at(comm))[source];
        vec_source = {dest_, -1, source_};
    }

    TRACE_MPI_COLLECTIVE(Sendrecv_replace, vec_source, buf, count, datatype, dest, sendtag, 
            source, recvtag, comm, status);
}



//===================================COLLECTIVE===================================================================

int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    std::vector<int> dests;

    int global_root = localToglobal.at(comm)[root];
    if (rank == root) {
        for (int i = 0; i < size; i++) {
            if (i != root) dests.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Bcast, dests, buffer, count, datatype, root, comm);
    } else {
        dests.push_back(-1);
        dests.push_back(global_root);
        TRACE_MPI_COLLECTIVE(Bcast, dests, buffer, count, datatype, root, comm);
    }
}


int MPI_Reduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    
    int global_root = localToglobal.at(comm)[root];
    if (rank == root) {
        int size;
        MPI_Comm_size(comm, &size);
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++) {
            if (i != root) sources.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Reduce, sources, sendbuf, recvbuf, count, datatype, op, root, comm);
    } else {
        TRACE_MPI_COLLECTIVE(Reduce, {global_root}, sendbuf, recvbuf, count, datatype, op, root, comm);
    }
}

int MPI_Gather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
              void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int global_root = localToglobal.at(comm)[root];
    if (rank == root){
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Gather, sources, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm);
    }
    else{
        TRACE_MPI_COLLECTIVE(Gather, {global_root}, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm);
    }
}

int MPI_Scatter(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
               void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int global_root = localToglobal.at(comm)[root];
    if (rank == root){
        std::vector<int> sources;
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Scatter, sources, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm);
    }
    else{
        std::vector<int> dests = {-1, global_root};
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

    int global_root = localToglobal.at(comm)[root];
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
        TRACE_MPI_COLLECTIVE(Gatherv, {global_root}, sendbuf, sendcount, sendtype,
               recvbuf, recvcounts, displs, recvtype, root, comm);
    }
}

int MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype, int root, MPI_Comm comm){
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int global_root = localToglobal.at(comm)[root];
    if (rank == root){
        std::vector<int> sources;
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(i);
        }
        TRACE_MPI_COLLECTIVE(Scatterv, sources, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount,
                recvtype, root, comm);
    }
    else{
        std::vector<int> dests = {-1, global_root};
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
    int global_root = localToglobal.at(comm)[root];
    if (rank == root) {
        for (int i = 0; i < size; i++) {
            if (i != root) dests.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Ibcast, dests, buffer, count, datatype, root, comm, request);
    } else {
        dests.push_back(-1);
        dests.push_back(global_root);
        TRACE_MPI_COLLECTIVE(Ibcast, dests, buffer, count, datatype, root, comm, request);
    }
}


int MPI_Ireduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm, MPI_Request* request) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    
    int global_root = localToglobal.at(comm)[root];
    if (rank == root) {
        int size;
        MPI_Comm_size(comm, &size);
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++) {
            if (i != root) sources.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Ireduce, sources, sendbuf, recvbuf, count, datatype, op, root, comm, request);
    } else {
        TRACE_MPI_COLLECTIVE(Ireduce, {global_root}, sendbuf, recvbuf, count, datatype, op, root, comm, request);
    }
}

int MPI_Igather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
              void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request){
    int rank, size;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    int global_root = localToglobal.at(comm)[root];
    if (rank == root){
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Igather, sources, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm, request);
    }
    else{
        TRACE_MPI_COLLECTIVE(Igather, {global_root}, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm, request);
    }
}

int MPI_Iscatter(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
               void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request){
    int rank, size;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    int global_root = localToglobal.at(comm)[root];
    if (rank == root){
        std::vector<int> sources;
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Iscatter, sources, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm, request);
    }
    else{
        std::vector<int> dests = {-1, global_root};
        TRACE_MPI_COLLECTIVE(Iscatter, dests, sendbuf, sendcount, sendtype,
              recvbuf, recvcount, recvtype, root, comm, request);
    }
}

int MPI_Igatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request){
    int rank, size;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    int global_root = localToglobal.at(comm)[root];
    if (rank == root){
        std::vector<int> sources;
        sources.push_back(-1);
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Igatherv, sources, sendbuf, sendcount, sendtype,
                recvbuf, recvcounts, displs, recvtype, root, comm, request);
    }
    else{
        TRACE_MPI_COLLECTIVE(Igatherv, {global_root}, sendbuf, sendcount, sendtype,
               recvbuf, recvcounts, displs, recvtype, root, comm, request);
    }
}

int MPI_Iscatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request){
    int rank, size;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    int global_root = localToglobal.at(comm)[root];
    if (rank == root){
        std::vector<int> sources;
        for (int i = 0; i < size; i++){
            if (i != root) sources.push_back(localToglobal.at(comm)[i]);
        }
        TRACE_MPI_COLLECTIVE(Iscatterv, sources, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount,
                recvtype, root, comm, request);
    }
    else{
        std::vector<int> dests = {-1, global_root};
        TRACE_MPI_COLLECTIVE(Iscatterv, dests, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount,
                recvtype, root, comm, request);
    }
}



//===================================ALL-OPERATIONS===================================================================

int MPI_Allreduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
                  MPI_Op op, MPI_Comm comm){
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(localToglobal.at(comm)[i]);
    }
    TRACE_MPI_COLLECTIVE(Allreduce, dests, sendbuf, recvbuf, count, datatype, op, comm);
}

int MPI_Allgather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                  void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm){
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(localToglobal.at(comm)[i]);
    }
    TRACE_MPI_COLLECTIVE(Allgather, dests, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}

int MPI_Allgatherv(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                   void* recvbuf, const int* recvcounts, const int* displs,
                   MPI_Datatype recvtype, MPI_Comm comm) {
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++) {
        if (i != rank) dests.push_back(localToglobal.at(comm)[i]);
    }
    TRACE_MPI_COLLECTIVE(Allgatherv, dests, sendbuf, sendcount, sendtype, 
                         recvbuf, recvcounts, displs, recvtype, comm);
}


int MPI_Alltoall(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                 void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm){
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(localToglobal.at(comm)[i]);
    }
    TRACE_MPI_COLLECTIVE(Alltoall, dests, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}


//===================================ALL-OPERATIONS-I===================================================================

int MPI_Iallreduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
                  MPI_Op op, MPI_Comm comm, MPI_Request* request){
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(localToglobal.at(comm)[i]);
    }
    TRACE_MPI_COLLECTIVE(Iallreduce, dests, sendbuf, recvbuf, count, datatype, op, comm, request);
}

int MPI_Iallgather(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                  void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request){
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(localToglobal.at(comm)[i]);
    }
    TRACE_MPI_COLLECTIVE(Iallgather, dests, sendbuf, sendcount, sendtype, recvbuf,
         recvcount, recvtype, comm, request);
}

int MPI_Ialltoall(const void* sendbuf, int sendcount, MPI_Datatype sendtype,
                 void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request){
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    std::vector<int> dests = {-2};
    for (int i = 0; i < size; i++){
        if (i != rank) dests.push_back(localToglobal.at(comm)[i]);
    }
    TRACE_MPI_COLLECTIVE(Ialltoall, dests, sendbuf, sendcount, sendtype, recvbuf,
         recvcount, recvtype, comm, request);
}



//===================================SIMPLE===================================================================

int MPI_Barrier(MPI_Comm comm) {
    TRACE_MPI_SIMPLE(Barrier, comm);
}

int MPI_Ibarrier(MPI_Comm comm, MPI_Request* request) {
    TRACE_MPI_SIMPLE(Ibarrier, comm, request);
}

int MPI_Finalize(void) {
    TRACE_MPI_SIMPLE(Finalize);
}

int MPI_Wait(MPI_Request *request, MPI_Status *status) {
    TRACE_MPI_SIMPLE(Wait, request, status);
}

int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[]) {
    TRACE_MPI_SIMPLE(Waitall, count, array_of_requests, array_of_statuses);
}

int MPI_Waitany(int count, MPI_Request array_of_requests[], int *index, MPI_Status *status) {
    TRACE_MPI_SIMPLE(Waitany, count, array_of_requests, index, status);
}

int MPI_Waitsome(int incount, MPI_Request array_of_requests[], int *outcount, int array_of_indices[], MPI_Status array_of_statuses[]) {
    TRACE_MPI_SIMPLE(Waitsome, incount, array_of_requests, outcount, array_of_indices, array_of_statuses);
}

int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status) {
    TRACE_MPI_SIMPLE(Test, request, flag, status);
}

int MPI_Testall(int count, MPI_Request array_of_requests[], int *flag, MPI_Status array_of_statuses[]) {
    TRACE_MPI_SIMPLE(Testall, count, array_of_requests, flag, array_of_statuses);
}

int MPI_Testany(int count, MPI_Request array_of_requests[], int *index, int *flag, MPI_Status *status) {
    TRACE_MPI_SIMPLE(Testany, count, array_of_requests, index, flag, status);
}

int MPI_Testsome(int incount, MPI_Request array_of_requests[], int *outcount, int array_of_indices[], MPI_Status array_of_statuses[]) {
    TRACE_MPI_SIMPLE(Testsome, incount, array_of_requests, outcount, array_of_indices, array_of_statuses);
}



//===================================COMMUNICATOR===================================================================

int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm){
    if (!global_collector) {
        return PMPI_Comm_create(comm, group, newcomm);
    }
    firstInit();

    long long start = global_collector->get_relative_time_us();
    int result = PMPI_Comm_create(comm, group, newcomm);
    long long end = global_collector->get_relative_time_us();

    std::vector<int> global_rank;
    if (result == MPI_SUCCESS && *newcomm != MPI_COMM_NULL) {
        getGlobalranks(*newcomm, global_rank, nullptr);
        localToglobal[*newcomm] = global_rank;
    }

    TraceItem item;
    item.name = "Comm_create";
    item.dests = global_rank;
    item.start = start;
    global_collector->push_back_begin(item);
    global_collector->push_back_end(end);

    return result;
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm){
    if (!global_collector) {
        return PMPI_Comm_dup(comm, newcomm);
    }
    firstInit();

    long long start = global_collector->get_relative_time_us();
    int result = PMPI_Comm_dup(comm, newcomm);
    long long end = global_collector->get_relative_time_us();

    std::vector<int> global_rank;
    if (result == MPI_SUCCESS && *newcomm != MPI_COMM_NULL) {
        getGlobalranks(*newcomm, global_rank, nullptr);
        localToglobal[*newcomm] = global_rank;
    }

    TraceItem item;
    item.name = "Comm_dup";
    item.dests = global_rank;
    item.start = start;
    global_collector->push_back_begin(item);
    global_collector->push_back_end(end);

    return result;
}

int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm){
    if (!global_collector) {
        return PMPI_Comm_split(comm, color, key, newcomm);
    }
    firstInit();

    long long start = global_collector->get_relative_time_us();
    int result = PMPI_Comm_split(comm, color, key, newcomm);
    long long end = global_collector->get_relative_time_us();

    std::vector<int> global_rank;
    if (result == MPI_SUCCESS && *newcomm != MPI_COMM_NULL) {
        getGlobalranks(*newcomm, global_rank, nullptr);
        localToglobal[*newcomm] = global_rank;
    }

    TraceItem item;
    item.name = "Comm_split";
    item.dests = global_rank;
    item.start = start;
    global_collector->push_back_begin(item);
    global_collector->push_back_end(end);

    return result;
}

int MPI_Comm_free(MPI_Comm *comm){
    if (!global_collector) {
        return PMPI_Comm_free(comm);
    }
    firstInit();

    MPI_Comm saved = *comm;

    std::vector<int> global_rank;
    if (saved != MPI_COMM_NULL && saved != MPI_COMM_WORLD && saved != MPI_COMM_SELF) {
        auto it = localToglobal.find(saved);
        if (it != localToglobal.end()) {
            global_rank = it->second;
        }
    }

    long long start = global_collector->get_relative_time_us();
    int result = PMPI_Comm_free(comm);
    long long end = global_collector->get_relative_time_us();

    TraceItem item;
    item.name = "Comm_free";
    item.dests = global_rank;
    item.start = start;
    global_collector->push_back_begin(item);
    global_collector->push_back_end(end);

    if (result == MPI_SUCCESS) {
        localToglobal.erase(saved);
    }

    return result;
}

