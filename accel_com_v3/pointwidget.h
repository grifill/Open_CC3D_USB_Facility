#ifndef POINTWIDGET_H
#define POINTWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QQueue>
#include <QMap>
#include <QTimer>

#define TAILSIZE 100

struct Tail
{
    QQueue<QPoint> pts;
    QPoint vel;
};

class PointWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PointWidget(QWidget *parent = 0);
    ~PointWidget();

    // Добавить новую движущуюся точку
    bool addTail(int ID, QPoint start, QPoint vel = QPoint(0, 0));
    // Добавить новую точку, возвращает ID
    int addTail(QPoint start, QPoint vel = QPoint(0, 0));
    // Обновить координаты точки
    void updatePosition(int ID, QPoint pos);
    // Обновить скорость точки
    void updateVelocity(int ID, QPoint vel);
    // Удалить точку
    void removeTail(int ID);
    // Удалить все точки
    void clear();

    // Запросить позицию точки
    QPoint position(int ID);
    // Запросить список ID
    QList<int>& getIDs();

protected:
    void paintEvent(QPaintEvent *);

signals:

    void pointUpdated(int ID, QPoint pos);

public slots:

private:
    QMap<int, Tail> tails;
    QList<int> IDs;
    QTimer tickTimer;

private slots:
    void tick();
};

#endif // POINTWIDGET_H
