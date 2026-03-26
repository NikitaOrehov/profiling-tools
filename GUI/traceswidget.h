#ifndef TRACESWIDGET_H
#define TRACESWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include "extractor.h"


class TracesWidget : public QWidget
{
    Q_OBJECT
private:
    const std::vector<std::vector<TraceItem>>& _traces;
    const std::vector<Arrow>& _arrows;
    //std::vector<int> _cache;
    extractor ext;

    const int height_item = 150;
    const double pixel_per_microsecond = 0.1;
    const int height_spacer = 50;
    const int _timeScaleHeight = 30;
    const int _timeTextHeight = 15;

    std::string timeFormat = "ms";

    double _currentScale = 1.0;
    long long _maxEnd = 0;


public:
    TracesWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void setScale(double scale);
    double getScale() const;
    long long getMaxEnd() const { return _maxEnd; }
    double getTracesWidth() const {return _maxEnd * pixel_per_microsecond;}
    double getTotalWidth() const { return sizeHint().width(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int calculateTotalHeight() const;
    long long calculateGridStep(long long timeRange, double pixelsPerUnit) const;
    QString formatTime(long long time) const;
    void drawArrow(QPainter &painter, const Arrow& arrow);
    QRectF getVisibleSceneRect() const;
    bool isVisibleArrow(QRectF& visibleRect, const Arrow& arrow) const;

signals:
};

#endif



