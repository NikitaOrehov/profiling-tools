#include "traceswidget.h"

TracesWidget::TracesWidget(QWidget *parent)
    : QWidget{parent}
    , ext("C:/Users/ornik/source/repos/ppc-2025-processes-informatics/build/bin/Traces6")
    , _traces(ext.GetTraces()) //D:/institute/profiling-tools/example_mpi_program/simple_method/Traces1
    , _arrows(ext.GetArrows()) //D:/institute/profiling-tools/example_mpi_program/bubble_sort/Traces1 +
                               //D:/institute/profiling-tools/example_mpi_program/radix_sort_double_batcher_merge/Traces1
                               //D:/institute/profiling-tools/example_mpi_program/global_opt/Traces1
{//D:/institute/profiling-tools/example_mpi_program/matrix_cannon/Traces1
//C:/Users/ornik/source/repos/ppc-2025-processes-informatics/build/bin/Traces1
    qDebug() << "test1.1\n";
    _maxEnd = ext.GetMaxEnd();
    _currentScale = 1.0;
    virtualTraceHeight =  _timeScaleHeight + _timeTextHeight + _traces.size() * (height_item + height_spacer);
    virtualTraceLength = _maxEnd * pixel_per_microsecond;

    setMouseTracking(true);
    qDebug() << "end constructor\n";
}


QSize TracesWidget::sizeHint() const {
    return this->size();
}

QSize TracesWidget::minimumSizeHint() const {
    return sizeHint();
}

void TracesWidget::setScale(double scale) {
    _currentScale = scale;
    virtualTraceLength = _maxEnd * pixel_per_microsecond * scale;
}

void TracesWidget::SetVisibleRect(QRectF rect){
    visibleRect_ = rect;
    update();
    //updateGeometry();
}

double TracesWidget::getScale() const { return _currentScale; }

void TracesWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    painter.translate(-visibleRect_.left(), -visibleRect_.top());
    painter.save();

    painter.setPen(QPen(Qt::black, 1));
    qDebug() << "virtualLenght: " << virtualTraceLength << "\n";
    qDebug() << "=================\nleft visible: " << visibleRect_.left() << "\nright visible: "<< visibleRect_.right() << "\n===================\n";
    qDebug() << "=================\nleft width: " << rect().width() << "\nright rect: "<< rect().height() << "\n===================\n\n";
    painter.drawLine(visibleRect_.left(), _timeScaleHeight, visibleRect_.right(), _timeScaleHeight);
    long long timeRange = _maxEnd;
    double pixelsPerUnit = getTracesWidth() / timeRange;
    long long gridStep = calculateGridStep(timeRange, pixelsPerUnit);

    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    for (long long time = gridStep; time <= _maxEnd; time += gridStep) { //можно ещё лучше !!?
        double x = time * pixel_per_microsecond * _currentScale;
        if (x < visibleRect_.topLeft().x()) continue;
        if (x > visibleRect_.topRight().x()) break;
        painter.drawLine(x, _timeScaleHeight - 10, x, _timeScaleHeight);

        QString timeText = formatTime(time);
        QRect textRect(x - 50, 5, 100, _timeTextHeight);
        painter.drawText(textRect, Qt::AlignCenter, timeText);
    }
    painter.restore();

    painter.save();
    painter.scale(_currentScale, 1.0);



    for (size_t number_trace = 0; number_trace < _traces.size(); ++number_trace) {
        int y_start = _timeScaleHeight + _timeTextHeight + number_trace * (height_item + height_spacer);

        if (y_start + height_item < visibleRect_.topLeft().y()) continue;
        if (y_start> visibleRect_.bottomLeft().y()) break;

        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(10, y_start + height_item / 2, QString("Trace %1").arg(number_trace + 1));

        painter.setPen(QPen(Qt::blue, 2 / _currentScale));
        painter.setBrush(QBrush(QColor(200, 220, 255)));

        for (const auto& item : _traces[number_trace]) {
            double x_start = item.start * pixel_per_microsecond;
            double item_width = (item.end - item.start) * pixel_per_microsecond;

            if ((x_start + item_width) * _currentScale < visibleRect_.topLeft().x()) continue;
            if (x_start * _currentScale > visibleRect_.topRight().x()) break;

            QRectF rectF(x_start, y_start, item_width, height_item);
            painter.drawRect(rectF);

            painter.save();
            painter.scale(1.0 / _currentScale, 1.0);

            QRectF textRectF(
                rectF.x() * _currentScale,
                rectF.y(),
                rectF.width() * _currentScale,
                rectF.height()
                );

            if (textRectF.width() > 30) {
                painter.setPen(QPen(Qt::black, 1));
                QString text = QString::fromStdString(item.name) +
                               QString("\n%1ms").arg(item.end - item.start);
                painter.drawText(textRectF, Qt::AlignCenter, text);
            }
            painter.restore();
        }
    }
    painter.restore();



    painter.save();
    painter.scale(_currentScale, 1.0);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    for (auto& it: _arrows){
        if (isVisibleArrow(visibleRect_, it)){
            drawArrow(painter, it);
        }
    }
    painter.restore();

    // ИНФОРМАЦИЯ О МАСШТАБЕ
    painter.setPen(Qt::red);
    painter.drawText(10, 20, QString("Scale: %1x").arg(_currentScale, 0, 'f', 2));
    painter.drawText(10, 40, QString("Max Time: %1 µs").arg(_maxEnd));
    painter.drawText(10, 60, QString("Traces: %1").arg(_traces.size()));
    painter.drawText(10, 80, QString("pixel/µs: %1").arg(pixel_per_microsecond, 0, 'f', 4));
}

bool TracesWidget::isVisibleArrow(QRectF& visibleRect, const Arrow& arrow) const {
    QPointF new_start = QPointF(arrow.start.x() * _currentScale, arrow.start.y());
    QPointF new_end = QPointF(arrow.end.x() * _currentScale, arrow.end.y());

    if (visibleRect.contains(new_start) || visibleRect.contains(new_end)) {
        return true;
    }

    QLineF line = QLineF(new_start, new_end);
    QPointF intersectionPoint;

    QLineF topLine(visibleRect.topLeft(), visibleRect.topRight());
    QLineF bottomLine(visibleRect.bottomLeft(), visibleRect.bottomRight());
    QLineF leftLine(visibleRect.topLeft(), visibleRect.bottomLeft());
    QLineF rightLine(visibleRect.topRight(), visibleRect.bottomRight());

    if (line.intersects(topLine, &intersectionPoint) == QLineF::BoundedIntersection ||
        line.intersects(bottomLine, &intersectionPoint) == QLineF::BoundedIntersection ||
        line.intersects(leftLine, &intersectionPoint) == QLineF::BoundedIntersection ||
        line.intersects(rightLine, &intersectionPoint) == QLineF::BoundedIntersection) {
        return true;
    }

    return false;
}

long long TracesWidget::calculateGridStep(long long timeRange, double pixelsPerUnit) const {
    double desiredPixelStep = 50.0;
    long long timeStep = desiredPixelStep / pixelsPerUnit;


    if (timeStep <= 5) return 5;
    if (timeStep <= 10) return 10;
    if (timeStep <= 50) return 50;
    if (timeStep <= 100) return 100;
    if (timeStep <= 500) return 500;
    if (timeStep <= 1000) return 1000;
    if (timeStep <= 5000) return 5000;

    for (long long i = 5000; i <= 1000000000000000; i += 1000){
        if (timeStep < i) return i;
    }
    return 1000000000000000;
}

QString TracesWidget::formatTime(long long time) const {
    if (timeFormat == "ns") {
        return QString("%1 ns").arg(time);
    } else if (timeFormat == "ms") {
        return QString("%1 ms").arg(time);
    } else if (timeFormat == "ml") {
        return QString("%1 ml").arg(time / 1000.0, 0, 'f', 1);
    } else {
        return QString("%1 s").arg(time / 1000000.0, 0, 'f', 2);
    }
}


void TracesWidget::drawArrow(QPainter &painter, const Arrow& arrow) {
    QPointF start = arrow.start;
    QPointF end = arrow.end;
    QPen originalPen = painter.pen();

    painter.save();

    QTransform transform = painter.transform();

    QPointF screenStart = transform.map(start);
    QPointF screenEnd = transform.map(end);

    painter.resetTransform();

    QPen arrowPen = originalPen;
    arrowPen.setWidthF(1.5);
    painter.setPen(arrowPen);

    painter.drawLine(screenStart, screenEnd);

    double arrowSize = 12.0;

    double angle = std::atan2(screenEnd.y() - screenStart.y(), screenEnd.x() - screenStart.x());

    QPointF arrowP1 = screenEnd - QPointF(
                          arrowSize * std::cos(angle - M_PI / 6),
                          arrowSize * std::sin(angle - M_PI / 6)
                          );
    QPointF arrowP2 = screenEnd - QPointF(
                          arrowSize * std::cos(angle + M_PI / 6),
                          arrowSize * std::sin(angle + M_PI / 6)
                          );

    painter.drawLine(screenEnd, arrowP1);
    painter.drawLine(screenEnd, arrowP2);

    if (arrow.twoSide) {
        QPointF startArrowP1 = screenStart + QPointF(
                                   arrowSize * std::cos(angle + M_PI / 6),
                                   arrowSize * std::sin(angle + M_PI / 6)
                                   );
        QPointF startArrowP2 = screenStart + QPointF(
                                   arrowSize * std::cos(angle - M_PI / 6),
                                   arrowSize * std::sin(angle - M_PI / 6)
                                   );

        painter.drawLine(screenStart, startArrowP1);
        painter.drawLine(screenStart, startArrowP2);
    }

    painter.restore();
    painter.setPen(originalPen);
}



