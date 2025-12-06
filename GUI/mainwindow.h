#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "tracesscrollarea.h"

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setGeometry(100, 100, 1200, 800);

        QWidget *centralWidget = new QWidget();
        setCentralWidget(centralWidget);

        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        // Панель управления
        QHBoxLayout *controlLayout = new QHBoxLayout();
        QPushButton *zoomInBtn = new QPushButton("Zoom In (Ctrl+Wheel)");
        QPushButton *zoomOutBtn = new QPushButton("Zoom Out (Ctrl+Wheel)");
        QPushButton *resetBtn = new QPushButton("Reset Zoom");
        QLabel *helpLabel = new QLabel("Shift+Drag: Pan | Wheel: Scroll");

        // controlLayout->addWidget(zoomInBtn);
        // controlLayout->addWidget(zoomOutBtn);
        // controlLayout->addWidget(resetBtn);
        // controlLayout->addWidget(helpLabel);
        controlLayout->addStretch();

        TracesScrollArea *scrollArea = new TracesScrollArea();

        layout->addLayout(controlLayout);
        layout->addWidget(scrollArea);

            // connect(zoomInBtn, &QPushButton::clicked, scrollArea, &TracesScrollArea::zoomIn);
            // connect(zoomOutBtn, &QPushButton::clicked, scrollArea, &TracesScrollArea::zoomOut);
            // connect(resetBtn, &QPushButton::clicked, scrollArea, &TracesScrollArea::resetZoom);
    }
};



