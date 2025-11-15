#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <algorithm>

struct TraceItem {
    std::string name;
    long long start;
    long long end;
    std::vector<int> dests;
};

class extractor
{
private:
    std::string _path;
    std::vector<std::vector<TraceItem>> _traces;
    std::vector<long long int> _starts;
    size_t _count_trace = 0;

public:
    extractor(std::string path) : _path(path){
        std::string file_name = _path + "/trace_rank_";
        while (std::filesystem::exists(file_name + std::to_string(_count_trace))){
           extract_data(file_name + std::to_string(_count_trace++));
        }
        _count_trace--;
        correct_data();
        print();
    }

    void extract_data(std::string path){
        std::ifstream file(path);

        if (!file.is_open()){
            std::cerr << "can not open file\n";
        }

        std::string first_line;
        if (!std::getline(file, first_line)) std::cerr << "file is empty\n";

        size_t pos = first_line.find(":");
        if (pos != std::string::npos){
            long long int start = std::stoll(first_line.substr(pos + 1));
            _starts.push_back(start);
        }

        std::string line;
        std::vector<TraceItem> trace;
        while (std::getline(file, line)) {
            TraceItem item;
            std::istringstream iss(line);
            if (!(iss >> item.name >> item.start >> item.end)) std::cerr << "can not parse data\n";

            int dest;
            while (iss >> dest) item.dests.push_back(dest);

            trace.push_back(item);

        }
        _traces.push_back(trace);

        file.close();
    }

    void correct_data(){
        auto min_it = std::min_element(_starts.begin(), _starts.end());
        size_t index = std::distance(_starts.begin(), min_it);

        for (size_t i = 0; i < _starts.size(); i++){
            if (i == index) continue;
            long long int offset = _starts[i] - _starts[index];
            for (size_t j = 0; j < _traces[i].size(); j++){
                _traces[i][j].start += offset;
                _traces[i][j].end += offset;
            }
        }
    }

    const std::vector<std::vector<TraceItem>>& GetTraces() const { return _traces;}
    long long int GetMaxEnd() const {
        long long int max = 0;
        for (size_t i = 0; i < _traces.size(); i++){
            if (_traces[i].back().end > max) max = _traces[i].back().end;
        }
        return max;
    }

    void print() const {
        std::cout << "=== Trace Data ===" << std::endl;

        std::cout << "System starts: ";
        for (size_t i = 0; i < _starts.size(); ++i) {
            std::cout << _starts[i];
            if (i < _starts.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl << std::endl;

        for (size_t i = 0; i < _traces.size(); ++i) {
            std::cout << "--- Trace " << i << " (System start: " << _starts[i] << ") ---" << std::endl;

            for (size_t j = 0; j < _traces[i].size(); ++j) {
                const auto& item = _traces[i][j];
                std::cout << "  " << item.name << " " << item.start << " " << item.end;

                if (!item.dests.empty()) {
                    std::cout << " [";
                    for (size_t k = 0; k < item.dests.size(); ++k) {
                        if (k > 0) std::cout << ", ";
                        std::cout << item.dests[k];
                    }
                    std::cout << "]";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }

};

#endif // EXTRACTOR_H
