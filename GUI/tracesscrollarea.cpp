#include "tracesscrollarea.h"
#include <QTimer>

TracesScrollArea::TracesScrollArea(QWidget *parent) : QWidget(parent) {
    tracesWidget = new TracesWidget(this);

    _hScrollBar = new QScrollBar(Qt::Horizontal, this);
    _vScrollBar = new QScrollBar(Qt::Vertical, this);

    _hScrollBar->setRange(0, 0);
    _vScrollBar->setRange(0, 0);
    _hScrollBar->setSingleStep(50);
    _vScrollBar->setSingleStep(50);
    _hScrollBar->setPageStep(100);
    _vScrollBar->setPageStep(100);

    connect(_hScrollBar, &QScrollBar::valueChanged, this, &TracesScrollArea::onHScrollChanged);
    connect(_vScrollBar, &QScrollBar::valueChanged, this, &TracesScrollArea::onVScrollChanged);

    setMouseTracking(true);

    QTimer::singleShot(0, this, [this]() {
        updateScrollBarRanges();
        updateVisibleRect();
    });
}

void TracesScrollArea::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    int scrollBarWidth = _vScrollBar->sizeHint().width();
    int scrollBarHeight = _hScrollBar->sizeHint().height();

    tracesWidget->setGeometry(0, 0,
                              width() - scrollBarWidth,
                              height() - scrollBarHeight);

    _hScrollBar->setGeometry(0, height() - scrollBarHeight,
                             width() - scrollBarWidth, scrollBarHeight);

    _vScrollBar->setGeometry(width() - scrollBarWidth, 0,
                             scrollBarWidth, height() - scrollBarHeight);

    updateScrollBarRanges();
    updateVisibleRect();
}

void TracesScrollArea::updateScrollBarRanges() {
    if (!tracesWidget) return;

    _virtualContentWidth = tracesWidget->getTracesWidth();
    _virtualContentHeight = tracesWidget->getTracesHeight();

    QSize viewportSize = tracesWidget->size();

    long long maxScrollX = _virtualContentWidth - viewportSize.width();
    long long maxScrollY = _virtualContentHeight - viewportSize.height();

    _virtualScrollX = qMax(0LL, qMin(_virtualScrollX, maxScrollX));
    _virtualScrollY = qMax(0LL, qMin(_virtualScrollY, maxScrollY));

    double scale = tracesWidget->getScale();
    int intMaxHScroll = (int)(maxScrollX / scale);
    int intMaxVScroll = (int)maxScrollY;

    const int MAX_SCROLL = 1000000000;
    intMaxHScroll = qMin(intMaxHScroll, MAX_SCROLL);
    intMaxVScroll = qMin(intMaxVScroll, MAX_SCROLL);
    intMaxHScroll = qMax(0, intMaxHScroll);
    intMaxVScroll = qMax(0, intMaxVScroll);

    _hScrollBar->blockSignals(true);
    _vScrollBar->blockSignals(true);

    _hScrollBar->setRange(0, intMaxHScroll);
    _vScrollBar->setRange(0, intMaxVScroll);

    int newHValue = (int)(_virtualScrollX / scale);
    int newVValue = (int)_virtualScrollY;

    newHValue = qBound(0, newHValue, intMaxHScroll);
    newVValue = qBound(0, newVValue, intMaxVScroll);

    _hScrollBar->setValue(newHValue);
    _vScrollBar->setValue(newVValue);

    _hScrollBar->blockSignals(false);
    _vScrollBar->blockSignals(false);

    _hScrollBar->setPageStep(viewportSize.width());
    _vScrollBar->setPageStep(viewportSize.height());
    _hScrollBar->setSingleStep(50);
    _vScrollBar->setSingleStep(50);
}

void TracesScrollArea::onHScrollChanged(int value) {
    if (_hScrollBar->signalsBlocked()) return;

    double scale = tracesWidget->getScale();
    long long newVirtualScrollX = (long long)(value * scale);

    QSize viewportSize = tracesWidget->size();
    long long maxScrollX = _virtualContentWidth - viewportSize.width();
    newVirtualScrollX = qMax(0LL, qMin(newVirtualScrollX, maxScrollX));

    if (_virtualScrollX != newVirtualScrollX) {
        _virtualScrollX = newVirtualScrollX;
        updateVisibleRect();
    }
}

void TracesScrollArea::onVScrollChanged(int value) {
    _virtualScrollY = value;
    updateVisibleRect();
}

void TracesScrollArea::updateVisibleRect() {
    if (!tracesWidget) return;

    QSize viewportSize = tracesWidget->size();

    QRectF visibleRect(_virtualScrollX, _virtualScrollY,
                       viewportSize.width(), viewportSize.height());

    tracesWidget->SetVisibleRect(visibleRect);
    tracesWidget->update();
}

void TracesScrollArea::zoomToMouse(double factor, const QPoint& globalPos) {
    if (!tracesWidget) return;

    QPoint viewportPos = tracesWidget->mapFromGlobal(globalPos);

    double oldScale = tracesWidget->getScale();
    double contentXBefore = (_virtualScrollX + viewportPos.x()) / oldScale;
    double contentYBefore = _virtualScrollY + viewportPos.y();

    double newScale = oldScale * factor;
    tracesWidget->setScale(newScale);

    _virtualContentWidth = tracesWidget->getTracesWidth();
    _virtualContentHeight = tracesWidget->getTracesHeight();
    QSize viewportSize = tracesWidget->size();

    long long newVirtualScrollX = (long long)(contentXBefore * newScale - viewportPos.x());
    long long newVirtualScrollY = (long long)(contentYBefore - viewportPos.y());

    long long maxScrollX = _virtualContentWidth - viewportSize.width();
    long long maxScrollY = _virtualContentHeight - viewportSize.height();

    newVirtualScrollX = qMax(0LL, qMin(newVirtualScrollX, maxScrollX));
    newVirtualScrollY = qMax(0LL, qMin(newVirtualScrollY, maxScrollY));

    _virtualScrollX = newVirtualScrollX;
    _virtualScrollY = newVirtualScrollY;

    updateScrollBarRanges();

    int newHScrollValue = (int)(_virtualScrollX / newScale);
    int newVScrollValue = (int)_virtualScrollY;

    _hScrollBar->blockSignals(true);
    _vScrollBar->blockSignals(true);

    _hScrollBar->setValue(newHScrollValue);
    _vScrollBar->setValue(newVScrollValue);

    _hScrollBar->blockSignals(false);
    _vScrollBar->blockSignals(false);

    updateVisibleRect();

}

void TracesScrollArea::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        double factor = (event->angleDelta().y() > 0) ? 1.25 : 0.8;
        zoomToMouse(factor, event->globalPosition().toPoint());
        event->accept();
    } else {
        int delta = event->angleDelta().y();
        if (event->modifiers() & Qt::ShiftModifier) {
            _hScrollBar->setValue(_hScrollBar->value() - delta);
        } else {
            _vScrollBar->setValue(_vScrollBar->value() - delta);
        }
        event->accept();
    }
}


void TracesScrollArea::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier) {
        _dragStartMousePos = event->pos();

        QPoint viewportPos = tracesWidget->mapFromParent(event->pos());
        double scale = tracesWidget->getScale();

        _dragStartContentPos = QPointF(
            (_virtualScrollX + viewportPos.x()) / scale,
            _virtualScrollY + viewportPos.y()
            );

        _isPanning = true;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void TracesScrollArea::mouseMoveEvent(QMouseEvent* event) {
    if (_isPanning) {

        QPoint currentMousePos = event->pos();

        QPoint delta = currentMousePos - _dragStartMousePos;

        double scale = tracesWidget->getScale();

        QPoint viewportPos = tracesWidget->mapFromParent(currentMousePos);

        long long newVirtualScrollX = (long long)(_dragStartContentPos.x() * scale - viewportPos.x());
        long long newVirtualScrollY = (long long)(_dragStartContentPos.y() - viewportPos.y());

        QSize viewportSize = tracesWidget->size();
        long long maxScrollX = _virtualContentWidth - viewportSize.width();
        long long maxScrollY = _virtualContentHeight - viewportSize.height();

        newVirtualScrollX = qMax(0LL, qMin(newVirtualScrollX, maxScrollX));
        newVirtualScrollY = qMax(0LL, qMin(newVirtualScrollY, maxScrollY));

        _virtualScrollX = newVirtualScrollX;
        _virtualScrollY = newVirtualScrollY;

        _hScrollBar->blockSignals(true);
        _vScrollBar->blockSignals(true);

        _hScrollBar->setValue((int)(_virtualScrollX / scale));
        _vScrollBar->setValue((int)_virtualScrollY);

        _hScrollBar->blockSignals(false);
        _vScrollBar->blockSignals(false);

        updateVisibleRect();

        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void TracesScrollArea::mouseReleaseEvent(QMouseEvent* event) {
    if (_isPanning && event->button() == Qt::LeftButton) {
        _isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}