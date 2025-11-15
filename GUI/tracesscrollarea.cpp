#include "tracesscrollarea.h"

TracesScrollArea::TracesScrollArea(QWidget *parent) : QScrollArea(parent) {
    tracesWidget = new TracesWidget();
    setWidget(tracesWidget);

    setWidgetResizable(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    tracesWidget->setMinimumSize(2000, 1000);

    setMouseTracking(true);
}

void TracesScrollArea::zoomIn() {
    zoomToMouse(1.25, QCursor::pos());
}

void TracesScrollArea::zoomOut() {
    zoomToMouse(0.8, QCursor::pos());
}

void TracesScrollArea::resetZoom() {
    tracesWidget->setScale(1.0);
}

void TracesScrollArea::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            zoomToMouse(1.25, event->globalPosition().toPoint());
        } else {
            zoomToMouse(0.8, event->globalPosition().toPoint());
        }
        event->accept();
    } else {
        QScrollArea::wheelEvent(event);
    }
}

void TracesScrollArea::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier) {
        _lastMousePos = event->pos();
        _isPanning = true;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QScrollArea::mousePressEvent(event);
    }
}

void TracesScrollArea::mouseMoveEvent(QMouseEvent *event) {
    if (_isPanning) {
        QPoint delta = event->pos() - _lastMousePos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        _lastMousePos = event->pos();
        event->accept();
    } else {
        QScrollArea::mouseMoveEvent(event);
    }
}

void TracesScrollArea::mouseReleaseEvent(QMouseEvent *event) {
    if (_isPanning && event->button() == Qt::LeftButton) {
        _isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QScrollArea::mouseReleaseEvent(event);
    }
}

void TracesScrollArea::zoomToMouse(double factor, const QPoint &globalPos) {
    if (!widget()) return;

    double oldScale = tracesWidget->getScale();
    QPoint viewportPos = viewport()->mapFromGlobal(globalPos);
    int oldHScroll = horizontalScrollBar()->value();
    int oldVScroll = verticalScrollBar()->value();

    double contentXBefore = (viewportPos.x() + oldHScroll) / oldScale;
    double contentYBefore = viewportPos.y() + oldVScroll;

    double newScale = oldScale * factor;

    double minScale = static_cast<double>(viewport()->width()) / tracesWidget->getTracesWidth();
    newScale = qBound(minScale, newScale, 15.0);

    tracesWidget->setScale(newScale);

    QSize newSize = tracesWidget->sizeHint();
    int maxWidth = tracesWidget->getTracesWidth() * newScale;

    tracesWidget->setFixedWidth(maxWidth);
    tracesWidget->setFixedHeight(newSize.height());

    double contentXAfter = contentXBefore * newScale;
    double contentYAfter = contentYBefore;

    int newHScroll = contentXAfter - viewportPos.x();
    int newVScroll = contentYAfter - viewportPos.y();

    newHScroll = qBound(0, newHScroll, maxWidth - viewport()->width());
    newVScroll = qMax(0, newVScroll);

    horizontalScrollBar()->setValue(newHScroll);
    verticalScrollBar()->setValue(newVScroll);

    updateScrollBars();

    // qDebug() << "Zoom:" << oldScale << "->" << newScale
    //          << "Min scale:" << minScale
    //          << "Widget width:" << maxWidth
    //          << "Viewport:" << viewport()->width()
    //          << "Scroll:" << newHScroll << "/" << horizontalScrollBar()->maximum();
}


void TracesScrollArea::updateScrollBars() {
    if (!widget()) return;

    QSize viewportSize = viewport()->size();

    int contentWidth = tracesWidget->getTracesWidth() * tracesWidget->getScale();
    int contentHeight = tracesWidget->sizeHint().height();

    horizontalScrollBar()->setRange(0, qMax(0, contentWidth - viewportSize.width()));
    verticalScrollBar()->setRange(0, qMax(0, contentHeight - viewportSize.height()));

    horizontalScrollBar()->setPageStep(viewportSize.width());
    verticalScrollBar()->setPageStep(viewportSize.height());

    // qDebug() << "Scrollbars - Content:" << contentWidth << "Viewport:" << viewportSize.width()
    //          << "Max scroll:" << horizontalScrollBar()->maximum();
}
