#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <fstream>
#include <filesystem>

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
    std::string FolderName;
    
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

    void CreateFolder(){
        FolderName = "Traces";
        int count = 1;
        if (!_rank_process){
            while (true){
                std::string temp_name = FolderName + std::to_string(count);
                if (!std::filesystem::exists(temp_name)){
                    std::filesystem::create_directory(temp_name);
                    break;
                }
                else{
                    count++;
                }
            }
        }
        MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        FolderName += std::to_string(count);
    }

    void CreateMetaFile(std::string FolderName){
        std::string file_name = FolderName + "/meta_file";
        std::ofstream file(file_name);

        file << "microseconds\n";

        file.close();
    }

    void CreateTraceFile(std::string FolderName){
        std::string file_name = FolderName + "/trace_rank_" + std::to_string(_rank_process);
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

    ~TraceCollector() {
        if (_rank_process == 0){
            CreateMetaFile(FolderName);
        }
        CreateTraceFile(FolderName);
    }
};