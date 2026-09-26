#pragma once


#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <mpi.h>

using time_metric = std::chrono::microseconds;

struct TraceItem {
    std::string name;
    long long start;
    std::vector<int> dests;
};

class TraceCollector {
private:
    int _rank_process;
    std::string FolderName;
    std::ofstream file;
    std::string file_name;
    
    std::chrono::system_clock::time_point _system_start;
    std::chrono::steady_clock::time_point _steady_start;

    
public:
    TraceCollector() {
        _system_start = std::chrono::system_clock::now();
        _steady_start = std::chrono::steady_clock::now();
        FolderName = "Traces";
    }


    void push_back_begin(const TraceItem& item) {
        file << item.name << " BEGIN " << item.start;
        if (!item.dests.empty()) {
            for (auto i: item.dests){
                file << " " << i;
            }
        }
        file.flush();
    }

    void push_back_end(long long end) {
        file << " END " << end;
        file << "\n"; 
        file.flush();
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
        PMPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        FolderName += std::to_string(count);

        if (_rank_process == 0){
            CreateMetaFile();
        }

        CreateTraceFile();
    }

    void CreateMetaFile(){
        std::string file_name = FolderName + "/meta_file";
        std::ofstream file(file_name);

        file << "microseconds\n";

        file.close();
    }

    void CreateTraceFile(){
        file_name = FolderName + "/trace_rank_" + std::to_string(_rank_process);
        file.open(file_name);

        file << "SYSTEM_START_US: " << std::chrono::duration_cast<time_metric>(
            _system_start.time_since_epoch()).count() << "\n";
    }

    ~TraceCollector() {
        file.close();
    }
};