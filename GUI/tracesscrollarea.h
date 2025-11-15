#ifndef TRACESSCROLLAREA_H
#define TRACESSCROLLAREA_H

#include <QScrollArea>
#include "traceswidget.h"

class TracesScrollArea : public QScrollArea
{
    Q_OBJECT
private:
    TracesWidget *tracesWidget;
    bool _isPanning = false;
    QPoint _lastMousePos;

public:
    TracesScrollArea(QWidget *parent = nullptr);

    void zoomIn();
    void zoomOut();
    void resetZoom();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void updateScrollBars();

private:
    void zoomToMouse(double factor, const QPoint &globalPos);
};

#endif
