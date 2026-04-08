#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <QPointF>
#include <QDebug>

struct TraceItem {
    std::string name;
    long long start;
    long long end;
    std::vector<int> dests;
};

struct Arrow{
    QPointF start;
    QPointF end;
    bool twoSide = 0;
    Arrow(QPointF Start, QPointF End, bool TwoSide) : start(Start), end(End), twoSide(TwoSide){}
};

class extractor
{
private:
    std::string _path;
    std::vector<std::vector<TraceItem>> _traces;
    std::vector<Arrow> arrows;
    std::vector<long long int> _starts;
    size_t _count_trace = 0;

    const int height_item = 100;
    const double pixel_per_microsecond = 0.1;
    const int height_spacer = 30;
    const int _timeScaleHeight = 30;
    const int _timeTextHeight = 15;




public:
    extractor(std::string path) : _path(path){
        std::string file_name = _path + "/trace_rank_";
        while (std::filesystem::exists(file_name + std::to_string(_count_trace))){
           extract_data(file_name + std::to_string(_count_trace++));
        }
        _count_trace--;
        correct_data();
        fulfill_arrows();
    }


    const std::vector<std::vector<TraceItem>>& GetTraces() const { return _traces;}
    long long int GetMaxEnd() const {
        long long int max = 0;
        for (size_t i = 0; i < _traces.size(); i++){
            if (_traces[i].back().end > max) max = _traces[i].back().end;
        }
        return max;
    }

    const std::vector<Arrow>& GetArrows() const {return arrows;}

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

private:
    void fulfill_arrows(){
        for (int fromTrace = 0; fromTrace < _traces.size(); fromTrace++){
            int y_start = _timeScaleHeight + _timeTextHeight + fromTrace * (height_item + height_spacer);
            for (auto& fromItem: _traces[fromTrace]){
                for (auto& toTrace: fromItem.dests){
                    if (toTrace == -1) {break;}
                    if (toTrace == -2) {continue;}
                    for (auto& toItem: _traces[toTrace]){
                        if (toItem.dests.empty()) continue;
                        bool check = (toItem.dests.front() == -2 || toItem.dests.front() == -1) &&
                                             (std::find(toItem.dests.begin(), toItem.dests.end(), fromTrace) != toItem.dests.end());
                        if (!check) continue;
                        if (fromItem.name != toItem.name){
                            if (!check_point_to_point(fromTrace, fromItem, toItem)){
                                continue;
                            }
                        }
                        double x_start = fromItem.start * pixel_per_microsecond;
                        double item_width = (fromItem.end - fromItem.start) * pixel_per_microsecond;
                        double offes_start = item_width * 0.1;
                        QPointF start = QPointF(x_start + offes_start, y_start + height_item / 2);

                        double item_width_dest = (toItem.end - toItem.start) * pixel_per_microsecond;
                        double offes_end = item_width_dest * 0.1 > 20 ? 20 : item_width_dest * 0.1;
                        int dest_y_start = _timeScaleHeight + _timeTextHeight + toTrace * (height_item + height_spacer);
                        QPointF end = QPointF(
                            toItem.start * pixel_per_microsecond + offes_end,
                            dest_y_start + height_item / 2
                            );

                        Arrow arrow(start, end, toItem.dests.front() == -2);
                        arrows.push_back(arrow);

                        auto it = std::find(toItem.dests.begin(), toItem.dests.end(), fromTrace);
                        toItem.dests.erase(it);
                        if (toItem.dests.size() == 1){
                            toItem.dests.clear();
                            toItem.dests.shrink_to_fit();
                        }
                        break;
                    }
                }
            }
        }
    }

    bool check_point_to_point(int fromTrace, TraceItem& fromItem, TraceItem& toItem){
        std::string to = toItem.name;
        std::string from = fromItem.name;
        if (to == "Irecv"){
            if (from == "Isend" || from == "Issend" || from == "Irsend" || from == "Ibsend"){
                if (fromTrace == toItem.dests[1]){
                    return 1;
                }
            }
        }
        else if (to == "Recv"){
            if (from == "Send" || from == "Ssend" || from == "Rsend" || from == "Bsend"){
                if (fromTrace == toItem.dests[1]){
                    return 1;
                }
            }
        }
        return 0;
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
        //std::vector<int> dests;
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

};

#endif // EXTRACTOR_H
