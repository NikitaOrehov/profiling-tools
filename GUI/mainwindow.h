#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class TracesScrollArea;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onOpenTrace();

private:
    void createMenu();

    TracesScrollArea* tracesArea = nullptr;
};

#endif


