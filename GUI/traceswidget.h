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
    extractor ext;
    const std::vector<std::vector<TraceItem>> _traces;
    const std::vector<Arrow> _arrows;
    QRectF visibleRect_;

    const int height_item = 100;
    const double pixel_per_microsecond = 0.1;
    const int height_spacer = 30;
    const int _timeScaleHeight = 30;
    const int _timeTextHeight = 15;

    std::string timeFormat = "ms";

    double _currentScale = 1.0;
    long long _maxEnd = 0;
    long long int virtualTraceHeight;
    long long int virtualTraceLength;


public:
    TracesWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void setScale(double scale);
    double getScale() const;
    double getTracesWidth() const {return virtualTraceLength;}
    double getTracesHeight() const {return virtualTraceHeight;}
    void SetVisibleRect(QRectF rect);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    long long calculateGridStep(long long timeRange, double pixelsPerUnit) const;
    QString formatTime(long long time) const;
    void drawArrow(QPainter &painter, const Arrow& arrow);
    bool isVisibleArrow(QRectF& visibleRect, const Arrow& arrow) const;

signals:
};

#endif



