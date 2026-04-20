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
    return app.exec();
}

