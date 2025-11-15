#include "traceswidget.h"

TracesWidget::TracesWidget(QWidget *parent)
    : QWidget{parent}
    , ext("D:/institute/profiling-tools/overloading/with system_clock/build/traces1")
{
    _traces = ext.GetTraces();
    _maxEnd = ext.GetMaxEnd();
    setMouseTracking(true);
}


QSize TracesWidget::sizeHint() const {
    int totalHeight = calculateTotalHeight();
    int totalWidth = getTracesWidth() * _currentScale;

    return QSize(totalWidth, totalHeight);
}

QSize TracesWidget::minimumSizeHint() const {
    return sizeHint();
}

void TracesWidget::setScale(double scale) {
    _currentScale = qBound(0.1, scale, 10.0);
    update();
    updateGeometry();
}

void TracesWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    painter.save();
    painter.scale(_currentScale, 1.0);

    painter.setPen(QPen(Qt::lightGray, 1));
    for (int x = 0; x <= _maxEnd * pixel_per_microsecond; x += 500) {
        painter.drawLine(x, 0, x, height());
    }

    painter.setPen(QPen(Qt::blue, 2));
    painter.setBrush(QBrush(QColor(200, 220, 255)));

    for (size_t number_trace = 0; number_trace < _traces.size(); ++number_trace) {
        int y_start = 20 + number_trace * (height_item + height_spacer);

        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(10, y_start + height_item / 2, QString("Trace %1").arg(number_trace + 1));

        painter.setPen(QPen(Qt::blue, 2));
        painter.setBrush(QBrush(QColor(200, 220, 255)));

        for (const auto& item : _traces[number_trace]) {
            int x_start = item.start * pixel_per_microsecond;
            int item_width = (item.end - item.start) * pixel_per_microsecond;

            if (item_width * _currentScale < 2) {
                item_width = 2 / _currentScale;
            }

            QRect Rect(x_start, y_start, item_width, height_item);
            painter.drawRect(Rect);

            if (item_width * _currentScale > 30) {
                painter.setPen(QPen(Qt::black, 1));
                QString text = QString::fromStdString(item.name) +
                               QString("\n%1-%2 µs").arg(item.start).arg(item.end);
                painter.drawText(Rect, Qt::AlignCenter, text);
                painter.setPen(QPen(Qt::blue, 2));
            }
        }
    }

    painter.restore();

    painter.setPen(Qt::red);
    painter.drawText(10, 20, QString("Scale: %1x").arg(_currentScale, 0, 'f', 2));
    painter.drawText(10, 40, QString("Max Time: %1 µs").arg(_maxEnd));
    painter.drawText(10, 60, QString("Traces: %1").arg(_traces.size()));
}

int TracesWidget::calculateTotalHeight() const {
    return 20 + _traces.size() * (height_item + height_spacer);
}

