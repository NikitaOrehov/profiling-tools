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
    std::unique_ptr<extractor> ext;
    const std::vector<std::vector<TraceItem>>* _traces = nullptr;
    const std::vector<Arrow>* _arrows = nullptr;
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
    explicit TracesWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void setScale(double scale);
    double getScale() const;
    double getTracesWidth() const;
    double getTracesHeight() const;
    void SetVisibleRect(QRectF rect);
    bool loadPath(const QString& path);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    long long calculateGridStep(long long timeRange, double pixelsPerUnit) const;
    QString formatTime(long long time) const;
    void drawArrow(QPainter& painter, const QPointF& start, const QPointF& end, bool double_direction);
    bool isVisibleArrow(QRectF& visibleRect, const Arrow& arrow) const;
    QColor colorForOperation(const std::string& name);

signals:
};

#endif



