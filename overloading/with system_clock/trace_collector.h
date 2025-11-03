#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <fstream>

using time_metric = std::chrono::microseconds;

struct TraceItem {
    std::string name;
    long long start;
    long long end; 
    std::vector<int> dests;
};

class TraceCollector {
private:
    std::vector<TraceItem> _trace;
    int _rank_process;
    
    std::chrono::system_clock::time_point _system_start;
    std::chrono::steady_clock::time_point _steady_start;
    
public:
    TraceCollector() {
        _system_start = std::chrono::system_clock::now();
        _steady_start = std::chrono::steady_clock::now();
    }

    void push_back(const TraceItem& item) {
        _trace.push_back(item);
    }

    void set_process(int rank) {
        _rank_process = rank;
    }

    long long get_relative_time_us() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<time_metric>(
            now - _steady_start).count();
    }

    ~TraceCollector() {
        std::string file_name = "trace_rank_" + std::to_string(_rank_process);
        std::ofstream file(file_name);
        
        file << "SYSTEM_START_US: " << std::chrono::duration_cast<time_metric>(
            _system_start.time_since_epoch()).count() << "\n";
        
        for (const auto& item : _trace) {
            file << item.name << " " << item.start << " " << item.end;
            if (!item.dests.empty()) {
                for (auto i: item.dests){
                    file << " " << i;
                }
            }
            file << "\n";
        }
        file.close();
    }
};