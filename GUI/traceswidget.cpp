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
    _currentScale = scale;
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
    painter.restore();

    painter.save();

    painter.fillRect(0, 0, width(), _timeScaleHeight, QColor(240, 240, 240));
    painter.setPen(QPen(Qt::black, 1));
    painter.drawLine(0, _timeScaleHeight, width(), _timeScaleHeight);

    long long timeRange = _maxEnd;
    double pixelsPerUnit = (getTracesWidth() * _currentScale) / timeRange;

    long long gridStep = calculateGridStep(timeRange, pixelsPerUnit);

    painter.setPen(QPen(Qt::black, 1));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    for (long long time = gridStep; time <= _maxEnd; time += gridStep) {
        double x = (time) * pixel_per_microsecond * _currentScale;
        painter.drawLine(x, _timeScaleHeight - 10, x, _timeScaleHeight);

        QString timeText = formatTime(time);
        QRect textRect(x - 50, 5, 100, _timeTextHeight);
        painter.drawText(textRect, Qt::AlignCenter, timeText);
    }

    painter.restore();

    painter.save();
    painter.scale(_currentScale, 1.0);
    painter.setPen(QPen(Qt::blue, 2));
    painter.setBrush(QBrush(QColor(200, 220, 255)));

    for (size_t number_trace = 0; number_trace < _traces.size(); ++number_trace) {
        int y_start = _timeScaleHeight + _timeTextHeight + number_trace * (height_item + height_spacer);

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

            painter.save();
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(QBrush(QColor(200, 220, 255)));
            for (int trace_dest: item.dests){
                for (int index = 0; index < _traces[trace_dest].size(); index++){
                    if (item.name == "Send" && _traces[trace_dest][index].name == "Recv" && !_traces[trace_dest][index].marks && _traces[trace_dest][index].dests.at(0) == number_trace){
                        QPoint start = QPoint(x_start + item_width, y_start + height_item / 2);
                        QPoint end = QPoint();
                        auto item_dest = _traces[trace_dest][index];
                        end.setX(item_dest.end * pixel_per_microsecond);
                        end.setY(_timeScaleHeight + _timeTextHeight + trace_dest * (height_item + height_spacer) + height_item / 2);
                        drawArrow(painter, start, end);
                        break;
                    }
                    if (item.name == _traces[trace_dest][index].name && !_traces[trace_dest][index].marks){
                        QPoint start = QPoint(x_start + item_width, y_start + height_item / 2);
                        QPoint end = QPoint();
                        auto item_dest = _traces[trace_dest][index];
                        end.setX(item_dest.end * pixel_per_microsecond);
                        end.setY(_timeScaleHeight + _timeTextHeight + trace_dest * (height_item + height_spacer) + height_item / 2);
                        drawArrow(painter, start, end);
                        break;
                    }
                }
            }
            painter.restore();
            painter.save();
            painter.scale(1.0 / _currentScale, 1.0);

            QRect textRect(
                Rect.x() * _currentScale,
                Rect.y(),
                Rect.width() * _currentScale,
                Rect.height()
                );

            if (textRect.width() > 30) {
                painter.setPen(QPen(Qt::black, 1));
                QString text = QString::fromStdString(item.name) +
                               QString("\n%1µs").arg(item.end - item.start);
                painter.drawText(textRect, Qt::AlignCenter, text);
            }

            painter.restore();
        }
    }

    painter.restore();

    painter.setPen(Qt::red);
    painter.drawText(10, 20, QString("Scale: %1x").arg(_currentScale, 0, 'f', 2));
    painter.drawText(10, 40, QString("Max Time: %1 µs").arg(_maxEnd));
    painter.drawText(10, 60, QString("Traces: %1").arg(_traces.size()));
}

int TracesWidget::calculateTotalHeight() const {
    return _timeScaleHeight + _timeTextHeight + _traces.size() * (height_item + height_spacer);
}


long long TracesWidget::calculateGridStep(long long timeRange, double pixelsPerUnit) const {
    double desiredPixelStep = 50.0;
    long long timeStep = desiredPixelStep / pixelsPerUnit;

    if (timeStep <= 10) return 10;
    if (timeStep <= 50) return 50;
    if (timeStep <= 100) return 100;
    if (timeStep <= 500) return 500;
    if (timeStep <= 1000) return 1000;
    if (timeStep <= 5000) return 5000;

    for (long long i = 10000; i <= 1000000000000000; i += 5000){
        if (timeStep < i) return i;
    }
    return 1000000000000000;
}

QString TracesWidget::formatTime(long long time) const {
    if (time < 1000) {
        return QString("%1 ns").arg(time);
    } else if (time < 1000000) {
        return QString("%1 µs").arg(time);
    } else if (time < 1000000000) {
        return QString("%1 ms").arg(time / 1000.0, 0, 'f', 1);
    } else {
        return QString("%1 s").arg(time / 1000000.0, 0, 'f', 2);
    }
}

void TracesWidget::drawArrow(QPainter &painter, const QPoint &start, const QPoint &end) {
    double arrowSize = 15;

    double angle = std::atan2(end.y() - start.y(), end.x() - start.x());

    QPoint p1 = end - QPoint(
                    arrowSize * std::cos(angle - M_PI / 6),
                    arrowSize * std::sin(angle - M_PI / 6)
                    );
    QPoint p2 = end - QPoint(
                    arrowSize * std::cos(angle + M_PI / 6),
                    arrowSize * std::sin(angle + M_PI / 6)
                    );
    painter.drawLine(start, end);
    painter.drawLine(end, p1);
    painter.drawLine(end, p2);
}

