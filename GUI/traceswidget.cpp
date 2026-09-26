#include "traceswidget.h"

#include <algorithm>
#include <cmath>

TracesWidget::TracesWidget(QWidget *parent)
    : QWidget{parent}
{
    _currentScale = 1.0;
    setMouseTracking(true);
    setAttribute(Qt::WA_StaticContents);

}

bool TracesWidget::loadPath(const QString& path) {
    try {
        auto newExt = std::make_unique<extractor>(path.toStdString());

        ext = std::move(newExt);
        _traces = &ext->GetTraces();
        _arrows = &ext->GetArrows();
        _maxEnd = ext->GetMaxEnd();

        _currentScale = 1.0;                       // ← добавить
        visibleRect_ = QRectF(0, 0, width(), height());

        virtualTraceHeight = _timeScaleHeight + _timeTextHeight
                             + _traces->size() * (height_item + height_spacer);
        virtualTraceLength = _maxEnd * pixel_per_microsecond * _currentScale;

        update();
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Failed to load trace:" << e.what();
        return false;
    }
}

QSize TracesWidget::sizeHint() const { return this->size(); }
QSize TracesWidget::minimumSizeHint() const { return sizeHint(); }
double TracesWidget::getTracesWidth() const {
    if (!_traces) return 1;
    return virtualTraceLength;
    }
double TracesWidget::getTracesHeight() const {
    if (!_traces) return 1;
    return virtualTraceHeight;
}

void TracesWidget::setScale(double scale) {
    _currentScale = scale;
    virtualTraceLength = _maxEnd * pixel_per_microsecond * scale;
}

void TracesWidget::SetVisibleRect(QRectF rect) {
    QRectF old = visibleRect_;
    visibleRect_ = rect;

    if (old == rect) {
        update();
        return;
    }

    const int dx = qRound(old.left() - rect.left());
    const int dy = qRound(old.top()  - rect.top());

    QRect dirty;
    if (dx > 0) {
        dirty = QRect(width() - dx, 0, dx, height());
    } else if (dx < 0) {
        dirty = QRect(0, 0, -dx, height());
    }

    if (dy > 0) {
        const QRect strip(0, height() - dy, width(), dy);
        dirty = dirty.isNull() ? strip : dirty.united(strip);
    } else if (dy < 0) {
        const QRect strip(0, 0, width(), -dy);
        dirty = dirty.isNull() ? strip : dirty.united(strip);
    }

    if (dirty.isNull()) {
        update();
    } else {
        update(dirty);
    }
}

double TracesWidget::getScale() const { return _currentScale; }

void TracesWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    if (!_traces || !_arrows) {
        painter.fillRect(rect(), Qt::white);
        painter.setPen(Qt::darkGray);
        painter.drawText(rect(), Qt::AlignCenter, "Трасса не загружена\nFile → Open Trace...");
        return;
    }

    const QRect clip = event->rect();

    const double screenLeft  = visibleRect_.left();
    const double screenTop   = visibleRect_.top();
    const double ppmScale    = pixel_per_microsecond * _currentScale;
    const double safePpm     = (ppmScale > 0) ? ppmScale : 1e-9;

    const double clipLeftScreen  = clip.left()  + screenLeft;
    const double clipRightScreen = clip.right() + screenLeft;

    {
        const int timeLineY = qRound(_timeScaleHeight - screenTop);

        painter.setPen(QPen(Qt::black, 1));
        if (timeLineY >= clip.top() && timeLineY <= clip.bottom()) {
            painter.drawLine(clip.left(), timeLineY, clip.right(), timeLineY);
        }

        QFont font = painter.font();
        font.setPointSize(8);
        painter.setFont(font);

        const long long timeRange = _maxEnd;
        const double pixelsPerUnit = safePpm;
        const long long gridStep = calculateGridStep(timeRange, pixelsPerUnit);

        for (long long time = 0; time <= _maxEnd; time += gridStep) {
            const double xWidget = time * safePpm - screenLeft;
            if (xWidget < clip.left() - 60) continue;
            if (xWidget > clip.right() + 60) break;

            const int x = qRound(xWidget);
            if (timeLineY >= clip.top() && timeLineY <= clip.bottom()) {
                painter.drawLine(x, timeLineY - 10, x, timeLineY);
            }

            const int textY = qRound(5 - screenTop);
            const QRect textRect(x - 50, textY, 100, _timeTextHeight);
            painter.drawText(textRect, Qt::AlignCenter, formatTime(time));
        }
    }

    for (size_t number_trace = 0; number_trace < _traces->size(); ++number_trace) {
        const int yData = _timeScaleHeight + _timeTextHeight + number_trace * (height_item + height_spacer);
        const int yWidget = qRound(yData - screenTop);

        if (yWidget + height_item < clip.top()) continue;
        if (yWidget > clip.bottom()) break;

        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(10, yWidget + height_item / 2,
                         QString("Trace %1").arg(number_trace + 1));

        const auto& trace = (*_traces)[number_trace];
        if (trace.empty()) continue;

        // Бинарный поиск первого элемента, у которого right >= clipLeftScreen.
        // Элементы в трассе отсортированы по времени, значит по end тоже.
        const size_t first = std::lower_bound(
                                 trace.begin(), trace.end(), clipLeftScreen,
                                 [safePpm](const TraceItem& item, double value) {
                                     return item.end * safePpm < value;
                                 }
                                 ) - trace.begin();

        for (size_t i = first; i < trace.size(); ++i) {
            const TraceItem& item = trace[i];

            const double xWidget = item.start * safePpm - screenLeft;
            double wWidget = (item.end - item.start) * safePpm;
            QColor color;
            if (item.finished || item.name == "Finalize"){
                color = colorForOperation(item.name);
            }
            else{
                color = QColor(0, 0, 255);
            }

            if (xWidget > clip.right()) break;
            if (wWidget < 1.0) wWidget = 1.0;

            painter.setPen(QPen(color, 1.0));
            painter.setBrush(QBrush(color));

            const QRectF rectF(xWidget, yWidget, wWidget, height_item);
            painter.drawRect(rectF);

            if (wWidget > 30) {
                painter.setPen(QPen(Qt::black, 1));
                const QString text = QString::fromStdString(item.name)
                                     + QString("\n%1ms").arg(item.end - item.start);
                painter.drawText(rectF, Qt::AlignCenter, text);
                painter.setPen(QPen(Qt::blue, qMax(1.0, 2.0 / _currentScale)));
            }
        }
    }

    if (!_arrows->empty()) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF clipF(clip);
        for (const Arrow& arrow : *_arrows) {
            QPointF s(arrow.start.x() * _currentScale - screenLeft,
                      arrow.start.y() - screenTop);
            QPointF e(arrow.end.x()   * _currentScale - screenLeft,
                      arrow.end.y()   - screenTop);

            QRectF bbox = QRectF(s, e).normalized().adjusted(-2, -2, 2, 2);
            if (!bbox.intersects(clip)) continue;

            if (s == e && !clipF.contains(s)) continue;

            QLineF line(s, e);
            QPointF dummy;
            const QRectF clipF(clip);
            if (!clipF.contains(s) && !clipF.contains(e)) {
                if (line.intersects(QLineF(clipF.topLeft(),    clipF.topRight()),    &dummy) != QLineF::BoundedIntersection &&
                    line.intersects(QLineF(clipF.bottomLeft(), clipF.bottomRight()), &dummy) != QLineF::BoundedIntersection &&
                    line.intersects(QLineF(clipF.topLeft(),    clipF.bottomLeft()),  &dummy) != QLineF::BoundedIntersection &&
                    line.intersects(QLineF(clipF.topRight(),   clipF.bottomRight()), &dummy) != QLineF::BoundedIntersection) {
                    continue;
                }
            }

            drawArrow(painter, s, e, arrow.twoSide);
        }
        painter.setRenderHint(QPainter::Antialiasing, false);
    }

    // ---------- Отладочная информация ----------
    painter.setPen(Qt::red);
    painter.drawText(10, 20,  QString("Scale: %1x").arg(_currentScale, 0, 'f', 2));
    painter.drawText(10, 40,  QString("Max Time: %1 µs").arg(_maxEnd));
    painter.drawText(10, 60,  QString("Traces: %1").arg(_traces->size()));
    painter.drawText(10, 80,  QString("pixel/µs: %1").arg(pixel_per_microsecond, 0, 'f', 4));
    painter.drawText(10, 100, QString("clip: %1,%2 %3x%4")
                                  .arg(clip.x()).arg(clip.y())
                                  .arg(clip.width()).arg(clip.height()));
}

long long TracesWidget::calculateGridStep(long long timeRange, double pixelsPerUnit) const {
    const double desiredPixelStep = 50.0;
    const long long timeStep = static_cast<long long>(desiredPixelStep / pixelsPerUnit);

    if (timeStep <= 5)    return 5;
    if (timeStep <= 10)   return 10;
    if (timeStep <= 50)   return 50;
    if (timeStep <= 100)  return 100;
    if (timeStep <= 500)  return 500;
    if (timeStep <= 1000) return 1000;
    if (timeStep <= 5000) return 5000;

    for (long long i = 5000; i <= 1000000000000000LL; i += 1000) {
        if (timeStep < i) return i;
    }
    return 1000000000000000LL;
}

QString TracesWidget::formatTime(long long time) const {
    if (timeFormat == "ns") {
        return QString("%1 ns").arg(time);
    } else if (timeFormat == "ms") {
        return QString("%1 ms").arg(time);
    } else if (timeFormat == "ml") {
        return QString("%1 ml").arg(time / 1000.0, 0, 'f', 1);
    }
    return QString("%1 s").arg(time / 1000000.0, 0, 'f', 2);
}

void TracesWidget::drawArrow(QPainter& painter,
                             const QPointF& start,
                             const QPointF& end,
                             bool double_direction) {
    QPen originalPen = painter.pen();

    QPen arrowPen(QColor(40, 70, 130));
    arrowPen.setWidthF(1.5);
    painter.setPen(arrowPen);

    painter.drawLine(start, end);

    const double arrowSize = 12.0;
    const double angle = std::atan2(end.y() - start.y(), end.x() - start.x());

    const QPointF arrowP1 = end - QPointF(
                                arrowSize * std::cos(angle - M_PI / 6),
                                arrowSize * std::sin(angle - M_PI / 6));
    const QPointF arrowP2 = end - QPointF(
                                arrowSize * std::cos(angle + M_PI / 6),
                                arrowSize * std::sin(angle + M_PI / 6));

    painter.drawLine(end, arrowP1);
    painter.drawLine(end, arrowP2);

    if (double_direction) {
        const QPointF sP1 = start + QPointF(
                                arrowSize * std::cos(angle + M_PI / 6),
                                arrowSize * std::sin(angle + M_PI / 6));
        const QPointF sP2 = start + QPointF(
                                arrowSize * std::cos(angle - M_PI / 6),
                                arrowSize * std::sin(angle - M_PI / 6));
        painter.drawLine(start, sP1);
        painter.drawLine(start, sP2);
    }

    painter.setPen(originalPen);
}

QColor TracesWidget::colorForOperation(const std::string& name) {
    // Коллективные: один-ко-многим / многие-к-одному
    if (name == "Bcast" || name == "Scatter" || name == "Scatterv" ||
        name == "Gather" || name == "Gatherv")
        return QColor(180, 220, 255);   // голубой

    // Reduce-операции
    if (name == "Reduce" || name == "Allreduce" ||
        name == "Reduce_scatter" || name == "Scan" || name == "Exscan")
        return QColor(255, 200, 140);   // оранжевый

    // All-операции (все-ко-всем)
    if (name == "Allgather" || name == "Allgatherv" ||
        name == "Alltoall"  || name == "Alltoallv"  || name == "Alltoallw")
        return QColor(200, 160, 255);   // сиреневый

    // Точка-точка
    if (name == "Send" || name == "Ssend" || name == "Rsend" || name == "Bsend")
        return QColor(255, 200, 200);   // красноватый
    if (name == "Recv")
        return QColor(200, 255, 200);   // зеленоватый
    if (name == "Sendrecv" || name == "Sendrecv_replace")
        return QColor(255, 240, 180);   // жёлтый

    // Неблокирующие
    if (name == "Isend" || name == "Irecv" ||
        name == "Wait"  || name == "Waitall" || name == "Waitany" || name == "Waitsome")
        return QColor(220, 200, 255);   // сиреневый бледный

    // Синхронизация
    if (name == "Barrier")
        return QColor(210, 210, 210);   // серый

    // Создание коммуникаторов
    if (name == "Comm_create" || name == "Comm_dup" || name == "Comm_split")
        return QColor(140, 230, 220);   // бирюзовый

    // Удаление коммуникатора
    if (name == "Comm_free" || name == "Comm_disconnect")
        return QColor(255, 160, 160);   // коралловый

    return QColor(230, 230, 230);       // прочее
}
