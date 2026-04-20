#ifndef TRACESSCROLLAREA_H
#define TRACESSCROLLAREA_H

#include <QWidget>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "traceswidget.h"

class TracesScrollArea : public QWidget
{
    Q_OBJECT
private:
    TracesWidget *tracesWidget;

    QScrollBar* _hScrollBar;
    QScrollBar* _vScrollBar;

    long long _virtualScrollX = 0;
    long long _virtualScrollY = 0;
    long long _virtualContentWidth = 0;
    long long _virtualContentHeight = 0;

    QPointF _dragStartContentPos;
    QPoint _dragStartMousePos;
    bool _isPanning = false;

public:
    TracesScrollArea(QWidget *parent = nullptr);
    ~TracesScrollArea() = default;

    void zoomIn();
    void zoomOut();
    void resetZoom();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onHScrollChanged(int value);
    void onVScrollChanged(int value);

private:
    void zoomToMouse(double factor, const QPoint &globalPos);
    void updateVisibleRect();
    void updateScrollBarRanges();
    void layoutWidgets();
};

#endif