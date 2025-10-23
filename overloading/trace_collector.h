#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <fstream>

struct item{
    std::string name;
    int start;
    int end;
};

class Trace_collector{
private:
    std::vector<item> _trace;
    int _rank_process;
public:
    std::chrono::steady_clock::time_point _start;
public:
    Trace_collector(){
        _start = std::chrono::steady_clock::now();
    }

    void push_back(item i){
        _trace.push_back(i);
    }

    void set_process(int rank){
        _rank_process = rank;
    }

    ~Trace_collector(){

        std::string file_name = "trace" + std::to_string(_rank_process);
        std::ofstream file(file_name);
        for (auto item: _trace){
            file << item.name << " " << item.start << " " << item.end << "\n";
        }
        file.close();
    }
};

Trace_collector collector;