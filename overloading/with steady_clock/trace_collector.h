#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <fstream>
#include <memory>

struct Item {
    std::string name;
    long long start_us;
    long long end_us;
    std::vector<int> dests;
};

class TraceCollector {
private:
    std::vector<Item> _trace;
    int _rank_process;
    std::chrono::steady_clock::time_point _synchronized_start;
    bool _initialized = false;

public:
    TraceCollector() = default;

    // Синхронизированная инициализация
    void initialize() {
        // СИНХРОНИЗАЦИЯ всех процессов
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Все процессы устанавливают _synchronized_start максимально одновременно
        _synchronized_start = std::chrono::steady_clock::now();
        
        MPI_Comm_rank(MPI_COMM_WORLD, &_rank_process);
        _initialized = true;
        
        MPI_Barrier(MPI_COMM_WORLD);
    }

    void push_back(const Item& item) {
        _trace.push_back(item);
    }

    long long get_relative_time_us() const {
        if (!_initialized) return 0;
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(
            now - _synchronized_start).count();
    }

    int get_rank() const {
        return _rank_process;
    }

    ~TraceCollector() {
        if (!_initialized) return;
        
        std::string file_name = "trace_rank_" + std::to_string(_rank_process) + ".txt";
        std::ofstream file(file_name);
        
        for (const auto& item : _trace) {
            file << item.name << " " 
                 << item.start_us << " " 
                 << item.end_us << "\n";
        }
        file.close();
    }
};


static std::unique_ptr<TraceCollector> global_collector;

#define TRACE_MPI_CALL(func_name, ...) \
    do { \
        if (!global_collector) break; \
        Item item; \
        item.name = #func_name; \
        item.start_us = global_collector->get_relative_time_us(); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        item.end_us = global_collector->get_relative_time_us(); \
        global_collector->push_back(item); \
        return result; \
    } while(0)

#define TRACE_MPI_CALL_WITH_RANKS(func_name, dest, source, ...) \
    do { \
        if (!global_collector) break; \
        Item item; \
        item.name = #func_name; \
        item.dest_rank = dest; \
        item.source_rank = source; \
        item.start_us = global_collector->get_relative_time_us(); \
        int result = PMPI_##func_name(__VA_ARGS__); \
        item.end_us = global_collector->get_relative_time_us(); \
        global_collector->push_back(item); \
        return result; \
    } while(0)


int MPI_Init(int *argc, char ***argv) {
    int result = PMPI_Init(argc, argv);
    
    global_collector = std::make_unique<TraceCollector>();
    global_collector->initialize();
    
    Item item;
    item.name = "MPI_Init";
    item.start_us = 0; 
    item.end_us = global_collector->get_relative_time_us();
    item.dest_rank = -1;
    item.source_rank = -1;
    global_collector->push_back(item);

    return result;
}

int MPI_Finalize(void) {
    TRACE_MPI_CALL(Finalize);
}

/* Точечные коммуникации */
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    TRACE_MPI_CALL_WITH_RANKS(Send, dest, -1, buf, count, datatype, dest, tag, comm);
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
    TRACE_MPI_CALL_WITH_RANKS(Recv, -1, source, buf, count, datatype, source, tag, comm, status);
}

int MPI_Isend(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request) {
    TRACE_MPI_CALL_WITH_RANKS(Isend, dest, -1, buf, count, datatype, dest, tag, comm, request);
}

int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request* request) {
    TRACE_MPI_CALL_WITH_RANKS(Irecv, -1, source, buf, count, datatype, source, tag, comm, request);
}

/* Коллективные операции */
int MPI_Barrier(MPI_Comm comm) {
    TRACE_MPI_CALL(Barrier, comm);
}

int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    TRACE_MPI_CALL_WITH_RANKS(Bcast, root, -1, buffer, count, datatype, root, comm);
}