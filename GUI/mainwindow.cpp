#include "mainwindow.h"
#include "tracesscrollarea.h"
#include "traceswidget.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    tracesArea = new TracesScrollArea(this);
    setCentralWidget(tracesArea);

    createMenu();
    resize(1200, 800);
}

void MainWindow::createMenu() {
    QMenu* fileMenu = menuBar()->addMenu("&File");

    QAction* openAct = fileMenu->addAction("&Open Trace...");
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenTrace);

    QAction* exitAct = fileMenu->addAction("E&xit");
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::onOpenTrace() {
    const QString dir = QFileDialog::getExistingDirectory(
        this, "Выберите папку с трассировкой", QDir::homePath());

    if (dir.isEmpty()) return;

    TracesWidget* w = tracesArea->getTracesWidget();
    if (!w) return;

    if (w->loadPath(dir)) {
        tracesArea->onTraceChanged();       // ← добавить
        statusBar()->showMessage("Загружено: " + dir, 3000);
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось загрузить трассировку из:\n" + dir);
    }
}