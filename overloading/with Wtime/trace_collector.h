#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <fstream>

#include <iomanip>
#include <sstream>

enum class TimeFormat {
    SECONDS,
    MILLISECONDS,
    MICROSECONDS,
    NANOSECONDS,
};

std::string format_time(double seconds, TimeFormat format) {
    std::ostringstream oss;
    oss << std::setprecision(15);
    
    switch (format) {
        case TimeFormat::SECONDS:
            oss << seconds;
            break;
            
        case TimeFormat::MILLISECONDS:
            oss << (seconds * 1e3);
            break;
            
        case TimeFormat::MICROSECONDS:
            oss << (seconds * 1e6);
            break;
            
        case TimeFormat::NANOSECONDS:
            oss << (seconds * 1e9);
            break;
    }
    
    return oss.str();
}


struct Item{
    std::string name;
    double start;
    double end;
    std::vector<int> dests;
};

class Trace_collector{
private:
    std::vector<Item> _trace;
    int _rank_process;
    double local_start;
    TimeFormat timeFormat = TimeFormat::MICROSECONDS;
public:
    Trace_collector() = default;

    void push_back(Item i){
        _trace.push_back(i);
    }

    void set_process(int rank){
        _rank_process = rank;
    }

    void set_start(double st){
        local_start = st;
    }

    double get_relative_time(){
        return MPI_Wtime() - local_start;
    }

    double get_start(){return local_start;}

    ~Trace_collector(){
        std::string file_name = "trace" + std::to_string(_rank_process);
        std::ofstream file(file_name);

        file << "ABSOLUTE_START: " << format_time(local_start, timeFormat) << "\n";

        for (auto Item: _trace){
            file << Item.name << " " << format_time(Item.start, timeFormat) << " " << format_time(Item.end, timeFormat) << "\n";
        }

        file.close();
    }
};

Trace_collector collector;