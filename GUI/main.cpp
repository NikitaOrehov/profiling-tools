#include "mainwindow.h"

#include <QApplication>

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "traceswidget.h"


int main(int argc, char *argv[])

{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    // для сортировки пузырьком
    // extractor ext("D:/institute/profiling-tools/example_mpi_program/bubble_sort/Traces1");
    // std::vector<TraceItem> trace3 = ext.GetTraces()[3];
    // double first_sendrecv = 91698;
    // double last_sendrecv = 155366;
    // double average_time_sendrecv = 0;
    // double max_time_sendrecv = 0;
    // int count = 0;
    // for (size_t i = 0; i < trace3.size(); i++){
    //     if (trace3[i].name == "Sendrecv"){
    //         count++;
    //         double time = trace3[i].end - trace3[i].start;
    //         if (time > max_time_sendrecv) max_time_sendrecv = time;
    //         average_time_sendrecv += time;
    //     }
    // }
    // double time = average_time_sendrecv;
    // double percent = time / (last_sendrecv - first_sendrecv);
    // average_time_sendrecv = average_time_sendrecv / count;

    // qDebug() << "count: " << count << "\nmax_time: " << max_time_sendrecv << "\naverage_time: " << average_time_sendrecv << "\n";
    // qDebug() << "percent: " << percent << "\n";
    // return 0;

    // для Кэннона
    // extractor ext("D:/institute/profiling-tools/example_mpi_program/matrix_cannon/Traces1");
    // std::vector<std::vector<TraceItem>> traces = ext.GetTraces();
    // std::vector<double> times(9);
    // qDebug() << "===============================\n";
    // for (size_t i = 0; i < traces.size(); i++) {
    //     double start_time = -1;
    //     double end_time = -1;
    //     double sum_mpi = 0;

    //     for (const auto& item : traces[i]) {
    //         if (start_time == -1 && item.name == "Sendrecv_replace") {
    //             start_time = item.start;
    //         }
    //         if (item.name == "Gather") {
    //             end_time = item.end;
    //         }
    //         if (start_time != -1 && (end_time == -1 || item.start <= end_time)) {
    //             sum_mpi += (item.end - item.start);
    //         }
    //     }

    //     double total_time = end_time - start_time;
    //     double mpi_percent = (total_time > 0) ? (sum_mpi / total_time * 100) : 0;

    //     qDebug() << "Process" << i
    //              << "Total:" << total_time << "us"
    //              << "MPI:" << sum_mpi << "us"
    //              << "MPI%:" << mpi_percent << "%";
    // }

    // extractor ext("D:/institute/profiling-tools/example_mpi_program/simple_method/Traces1");
    // std::vector<std::vector<TraceItem>> traces = ext.GetTraces();

    // for (size_t i = 0; i < traces.size(); i++) {
    //     double first_allgatherv_start = -1;
    //     double last_allreduce_end = -1;
    //     double sum_mpi_time = 0.0;
    //     double sum_allgatherv_time = 0.0, max_allgatherv = 0.0;
    //     double sum_allreduce_time = 0.0, max_allreduce = 0.0;
    //     int count_allgatherv = 0, count_allreduce = 0;

    //     for (const auto& item : traces[i]) {
    //         if (item.name == "Allgatherv" || item.name == "Allreduce") {
    //             double dur = item.end - item.start;
    //             sum_mpi_time += dur;
    //             if (first_allgatherv_start == -1) first_allgatherv_start = item.start;
    //             last_allreduce_end = item.end;

    //             if (item.name == "Allgatherv") {
    //                 count_allgatherv++;
    //                 sum_allgatherv_time += dur;
    //                 max_allgatherv = std::max(max_allgatherv, dur);
    //             } else {
    //                 count_allreduce++;
    //                 sum_allreduce_time += dur;
    //                 max_allreduce = std::max(max_allreduce, dur);
    //             }
    //         }
    //     }

    //     double total_time = last_allreduce_end - first_allgatherv_start;
    //     double mpi_percent = (total_time > 0) ? (sum_mpi_time / total_time * 100) : 0;
    //     double avg_allgatherv = count_allgatherv ? (sum_allgatherv_time / count_allgatherv) : 0;
    //     double avg_allreduce = count_allreduce ? (sum_allreduce_time / count_allreduce) : 0;

    //     qDebug() << "Process" << i
    //              << "Total time (pure):" << total_time << "us"
    //              << "MPI time:" << sum_mpi_time << "us"
    //              << "MPI%:" << mpi_percent << "%"
    //              << "\nAllgatherv count:" << count_allgatherv << "avg:" << avg_allgatherv << "us max:" << max_allgatherv << "us"
    //              << "\nAllreduce count:" << count_allreduce << "avg:" << avg_allreduce << "us max:" << max_allreduce << "us";
    // }

    // для метода простой итерации
    // extractor ext("D:/institute/profiling-tools/example_mpi_program/simple_method/Traces1");
    // std::vector<std::vector<TraceItem>> traces = ext.GetTraces();

    // qDebug() << "===============================\n";
    // for (size_t rank = 0; rank < traces.size(); ++rank) {
    //     double iter_start = -1;
    //     double iter_end = -1;
    //     double sum_mpi = 0;
    //     int count_allgatherv = 0;
    //     int count_allreduce = 0;
    //     double sum_allgatherv = 0;
    //     double sum_allreduce = 0;
    //     double max_allgatherv = 0;
    //     double max_allreduce = 0;

    //     for (const auto& item : traces[rank]) {

    //         if (iter_start == -1 && item.name == "Scatterv") {
    //             iter_start = item.start;
    //         }

    //         if (item.name == "Allreduce") {
    //             iter_end = item.end;
    //         }


    //         if (iter_start != -1 && (iter_end == -1 || item.start <= iter_end)) {
    //             double dur = item.end - item.start;
    //             sum_mpi += dur;

    //             if (item.name == "Allgatherv") {
    //                 count_allgatherv++;
    //                 sum_allgatherv += dur;
    //                 if (dur > max_allgatherv) max_allgatherv = dur;
    //             }
    //             if (item.name == "Allreduce") {
    //                 count_allreduce++;
    //                 sum_allreduce += dur;
    //                 if (dur > max_allreduce) max_allreduce = dur;
    //             }
    //         }
    //     }

    //     double total_time = iter_end - iter_start;
    //     double mpi_percent = (total_time > 0) ? (sum_mpi / total_time * 100) : 0;
    //     double avg_allgatherv = (count_allgatherv > 0) ? (sum_allgatherv / count_allgatherv) : 0;
    //     double avg_allreduce = (count_allreduce > 0) ? (sum_allreduce / count_allreduce) : 0;

    //     qDebug() << "Process" << rank
    //              << "| Total:" << total_time / 1000.0 << "ms"
    //              << "| MPI:" << sum_mpi / 1000.0 << "ms"
    //              << "| MPI%:" << mpi_percent << "%"
    //              << "| Allgatherv count:" << count_allgatherv
    //              << "avg:" << avg_allgatherv << "us"
    //              << "max:" << max_allgatherv << "us"
    //              << "| Allreduce count:" << count_allreduce
    //              << "avg:" << avg_allreduce << "us"
    //              << "max:" << max_allreduce << "us";
    // }


    // return 0;
    return app.exec();
}

