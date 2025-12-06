#include "mainwindow.h"

#include <QApplication>

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "traceswidget.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}



// // main.cpp
// #include <QApplication>
// #include <QMainWindow>
// #include <QWidget>
// #include <QPainter>
// #include <QMouseEvent>
// #include <QWheelEvent>
// #include <QTransform>

// class ZoomableWidget : public QWidget
// {
//     Q_OBJECT

// public:
//     ZoomableWidget(QWidget *parent = nullptr) : QWidget(parent), scale(1.0), offset(0, 0), isDragging(false)
//     {
//         setMouseTracking(true);
//     }

// protected:
//     void paintEvent(QPaintEvent *event) override
//     {
//         QPainter painter(this);
//         painter.setRenderHint(QPainter::Antialiasing);

//         // Сохраняем текущее состояние painter
//         painter.save();

//         // Применяем трансформацию (масштабирование и смещение)
//         painter.translate(width() / 2 + offset.x(), height() / 2 + offset.y());
//         painter.scale(scale, scale);

//         // Рисуем координатную сетку для наглядности
//         drawGrid(painter);

//         // Рисуем основную стрелку (будет масштабироваться)
//         drawScalableArrow(painter);

//         // Рисуем фиксированные стрелки в точках сцены
//         drawFixedSizeArrowsAtScenePoints(painter);

//         // Восстанавливаем состояние painter (убираем трансформацию масштабирования)
//         painter.restore();

//         // Рисуем стрелку постоянного размера (в координатах экрана)
//         drawFixedSizeArrow(painter);

//         // Рисуем информационный текст
//         drawInfoText(painter);
//     }

//     void wheelEvent(QWheelEvent *event) override
//     {
//         double zoomFactor = 1.1;
//         if (event->angleDelta().y() > 0) {
//             // Увеличиваем масштаб
//             scale *= zoomFactor;
//         } else {
//             // Уменьшаем масштаб
//             scale /= zoomFactor;
//         }

//         // Ограничиваем масштаб
//         scale = qBound(0.01, scale, 100.0);

//         update();
//         event->accept();
//     }

//     void mousePressEvent(QMouseEvent *event) override
//     {
//         if (event->button() == Qt::LeftButton) {
//             isDragging = true;
//             lastMousePos = event->pos();
//         }
//     }

//     void mouseMoveEvent(QMouseEvent *event) override
//     {
//         if (isDragging) {
//             QPoint delta = event->pos() - lastMousePos;
//             offset += delta;
//             lastMousePos = event->pos();
//             update();
//         }
//     }

//     void mouseReleaseEvent(QMouseEvent *event) override
//     {
//         if (event->button() == Qt::LeftButton) {
//             isDragging = false;
//         }
//     }

// private:
//     double scale;
//     QPoint offset;
//     bool isDragging;
//     QPoint lastMousePos;

//     void drawGrid(QPainter &painter)
//     {
//         painter.setPen(QPen(QColor(200, 200, 200), 0.5));

//         // Рисуем сетку
//         int gridSize = 50;
//         for (int x = -500; x <= 500; x += gridSize) {
//             painter.drawLine(x, -500, x, 500);
//         }
//         for (int y = -500; y <= 500; y += gridSize) {
//             painter.drawLine(-500, y, 500, y);
//         }

//         // Рисуем оси
//         painter.setPen(QPen(Qt::black, 1));
//         painter.drawLine(-500, 0, 500, 0); // Ось X
//         painter.drawLine(0, -500, 0, 500); // Ось Y
//     }

//     void drawScalableArrow(QPainter &painter)
//     {
//         // Эта стрелка будет масштабироваться вместе с zoom
//         painter.setPen(QPen(Qt::blue, 2));
//         painter.setBrush(QBrush(QColor(100, 100, 255, 100)));

//         // Рисуем стрелку в центре координат (масштабируемую)
//         QPointF points[7] = {
//             QPointF(0, 0),
//             QPointF(40, 0),
//             QPointF(40, -15),
//             QPointF(60, 0),
//             QPointF(40, 15),
//             QPointF(40, 0),
//             QPointF(0, 0)
//         };

//         painter.drawPolyline(points, 7);

//         // Подпись для масштабируемой стрелки
//         painter.setPen(Qt::blue);
//         painter.drawText(0, -30, "Масштабируемая стрелка");
//     }

//     void drawFixedSizeArrow(QPainter &painter)
//     {
//         // Эта стрелка всегда будет одного размера в пикселях экрана
//         painter.setPen(QPen(Qt::red, 2));
//         painter.setBrush(QBrush(QColor(255, 100, 100, 150)));

//         // Позиция фиксированной стрелки (правый верхний угол)
//         // Координаты в пикселях экрана, не зависят от масштаба
//         int arrowX = width() - 100;
//         int arrowY = 50;

//         // Размеры стрелки (фиксированные в пикселях)
//         int arrowLength = 60;
//         int arrowHeadWidth = 20;

//         // Рисуем стрелку в координатах экрана
//         QPointF points[7] = {
//             QPointF(arrowX, arrowY),
//             QPointF(arrowX + arrowLength - arrowHeadWidth, arrowY),
//             QPointF(arrowX + arrowLength - arrowHeadWidth, arrowY - arrowHeadWidth/2),
//             QPointF(arrowX + arrowLength, arrowY),
//             QPointF(arrowX + arrowLength - arrowHeadWidth, arrowY + arrowHeadWidth/2),
//             QPointF(arrowX + arrowLength - arrowHeadWidth, arrowY),
//             QPointF(arrowX, arrowY)
//         };

//         painter.drawPolyline(points, 7);

//         // Подпись для фиксированной стрелки
//         painter.setPen(Qt::red);
//         painter.drawText(arrowX, arrowY + 40, "Фиксированный размер (экран)");
//     }

//     void drawFixedSizeArrowsAtScenePoints(QPainter &painter)
//     {
//         // Рисуем несколько фиксированных стрелок в разных точках сцены

//         // Стрелка в точке (100, 100) сцены
//         drawFixedSizeArrowAtScenePoint(painter, QPointF(100, 100), Qt::green);

//         // Стрелка в точке (-150, -80) сцены
//         drawFixedSizeArrowAtScenePoint(painter, QPointF(-150, -80), Qt::magenta);

//         // Стрелка в точке (200, -120) сцены
//         drawFixedSizeArrowAtScenePoint(painter, QPointF(200, -120), Qt::darkYellow);

//         // Стрелка на пересечении сетки
//         drawFixedSizeArrowAtScenePoint(painter, QPointF(250, 250), Qt::cyan);
//     }

//     void drawFixedSizeArrowAtScenePoint(QPainter &painter, const QPointF &scenePos, const QColor &color)
//     {
//         painter.save();

//         // Создаем трансформацию для преобразования координат сцены в экранные
//         QTransform transform;
//         transform.translate(width() / 2 + offset.x(), height() / 2 + offset.y());
//         transform.scale(scale, scale);
//         QPointF screenPos = transform.map(scenePos);

//         // Восстанавливаем исходные трансформации для рисования в координатах экрана
//         painter.resetTransform();

//         // Рисуем стрелку фиксированного размера в координатах экрана
//         int arrowSize = 24; // фиксированный размер в пикселях

//         QPointF points[4] = {
//             screenPos + QPointF(arrowSize, 0),                    // кончик стрелки
//             screenPos + QPointF(-arrowSize/3, -arrowSize/3),      // левый верхний
//             screenPos + QPointF(0, 0),                           // центр (основание)
//             screenPos + QPointF(-arrowSize/3, arrowSize/3)       // левый нижний
//         };

//         painter.setPen(QPen(color, 2));
//         painter.setBrush(QBrush(QColor(color.red(), color.green(), color.blue(), 100)));
//         painter.drawPolygon(points, 4);

//         // Рисуем точку в центре стрелки
//         painter.setBrush(QBrush(color));
//         painter.drawEllipse(screenPos, 3, 3);

//         // Подпись (тоже фиксированного размера)
//         painter.setPen(color);
//         QFont font = painter.font();
//         font.setPointSize(8);
//         painter.setFont(font);
//         painter.drawText(screenPos + QPointF(arrowSize + 5, 5),
//                          QString("(%1,%2)").arg(scenePos.x()).arg(scenePos.y()));

//         painter.restore();
//     }

//     void drawInfoText(QPainter &painter)
//     {
//         painter.setPen(Qt::black);
//         painter.drawText(10, 30, QString("Масштаб: %1x").arg(scale, 0, 'f', 2));
//         painter.drawText(10, 50, "Колесико мыши - zoom");
//         painter.drawText(10, 70, "ЛКМ + перемещение - панорамирование");
//         painter.drawText(10, 90, "Синяя стрелка - масштабируется с zoom");
//         painter.drawText(10, 110, "Красная стрелка - фиксированная на экране");
//         painter.drawText(10, 130, "Цветные стрелки - фиксированные в точках сцены");

//         // Дополнительная информация о размерах
//         painter.drawText(10, 160, QString("Размер цветных стрелок: 24 пикселя (всегда)"));
//         painter.drawText(10, 180, "Они привязаны к координатам сцены, но не масштабируются");
//     }
// };

// class MainWindow : public QMainWindow
// {
//     Q_OBJECT

// public:
//     MainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
//     {
//         setWindowTitle("Фиксированные стрелки в точках сцены");
//         setMinimumSize(800, 600);

//         ZoomableWidget *widget = new ZoomableWidget(this);
//         setCentralWidget(widget);
//     }
// };

// int main(int argc, char *argv[])
// {
//     QApplication app(argc, argv);

//     MainWindow window;
//     window.show();

//     return app.exec();
// }

// #include "main.moc"

