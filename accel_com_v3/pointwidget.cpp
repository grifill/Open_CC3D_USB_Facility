#include "pointwidget.h"
#include <math.h>
#include <QtDebug>
#include <time.h>
#include <stdlib.h>

typedef QMap<int, Tail>::iterator Iter;
typedef QMap<int, Tail>::const_iterator CIter;

PointWidget::PointWidget(QWidget *parent)
    : QWidget(parent)
{
    connect(&tickTimer, SIGNAL(timeout()), SLOT(tick()));
    tickTimer.start(50);
    srand(time(0));
}
//------------------------------------------------------------------------------
PointWidget::~PointWidget()
{
    clear();
}
//------------------------------------------------------------------------------
void PointWidget::clear()
{
    Iter i = tails.begin();
    while(i != tails.constEnd())
    {
        i.value().pts.clear();
        i++;
    }

    tails.clear();
}
//------------------------------------------------------------------------------
void PointWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int W = width();
    int H = height();
    int S;
    if(W>H)
        S=H-10;
    else
        S=W-10;

    QPoint mid = QPoint(W/2, H/2);

    QColor grid_color(128, 128, 128, 128);
    QPen grid_pen(grid_color);
    QColor bg_color(255, 255, 255);
    QBrush bg_brush(bg_color);

    // Заливка фона
    p.setPen(Qt::NoPen);
    p.setBrush(bg_brush);
    p.drawRect(0, 0, W, H);

    // Отрисовка сетки
    p.setPen(grid_pen);
    for(int a=0; a<11; a++)
    {
        int x = mid.x() - S/2 + a*S/10;
        int y = mid.y() - S/2 + a*S/10;
        p.drawLine(W/2-S/2, y, W/2+S/2, y);
        p.drawLine(x, H/2-S/2, x, H/2+S/2);
    }

    // Отрисовка точек
    CIter ci = tails.cbegin();

    for(ci=tails.cbegin(); ci!=tails.cend(); ci++)
    {
        const Tail& t = ci.value();
        if(t.pts.isEmpty())
            continue;

        QColor tail_color;
        int ID = ci.key();
        if(ID==0)
            tail_color.setRgb(0, 0, 0);
        else
            tail_color.setHsv((ID*36)%360, 100, 100);
        p.setPen(QPen(tail_color));
        p.setBrush(QBrush(tail_color));
        QPoint pt = t.pts.first();
        int x;
        int y;
        for(int a=1; a<t.pts.size(); a++)
        {
            tail_color.setAlpha(floor((float)a/t.pts.size()*255));
            p.setPen(QPen(tail_color));
            pt = t.pts[a];
            int nx=mid.x() + floor((float)pt.x()*S/100);
            int ny=mid.y() - floor((float)pt.y()*S/100);
            if(a>0)
                p.drawLine(x, y, nx, ny);
            x = nx;
            y = ny;
        }

        p.drawRect(x-2, y-2, 5, 5);
    }
}
//------------------------------------------------------------------------------
void PointWidget::addTail(int ID, QPoint start, QPoint vel)
{
    if(tails.contains(ID))
        return;

    Tail t;
    t.pts.append(start);
    t.vel = vel;
    tails[ID] = t;
}
//------------------------------------------------------------------------------
int PointWidget::addTail(QPoint start, QPoint vel)
{
    int ID=0;
    while(tails.contains(ID++));
    addTail(ID, start, vel);
    return ID;
}
//------------------------------------------------------------------------------
void PointWidget::updatePosition(int ID, QPoint pos)
{
    if(!tails.contains(ID))
        return;

    Tail& t = tails[ID];
    t.pts.enqueue(pos);
    while(t.pts.size() > TAILSIZE)
        t.pts.dequeue();
}
//------------------------------------------------------------------------------
void PointWidget::updateVelocity(int ID, QPoint vel)
{
    if(!tails.contains(ID))
        return;

    Tail& t = tails[ID];
    t.vel = vel;
}
//------------------------------------------------------------------------------
void PointWidget::removeTail(int ID)
{
    tails.remove(ID);
}
//------------------------------------------------------------------------------
QPoint PointWidget::position(int ID)
{
    if(!tails.contains(ID))
        return QPoint();

    return tails[ID].pts.last();
}
//------------------------------------------------------------------------------
QList<int> &PointWidget::getIDs()
{
    IDs.clear();

    Iter i = tails.begin();
    while(i != tails.constEnd())
    {
        IDs.append(i.key());
        i++;
    }

    return IDs;
}
//------------------------------------------------------------------------------
void PointWidget::tick()
{
    Iter i = tails.begin();
    while(i != tails.constEnd())
    {
        Tail& t = i.value();
        QPoint pt = t.pts.last() + t.vel;
        if(abs(pt.x()) > 50 || abs(pt.y()) > 50)
        {
            pt.setX(floor((float)rand() / RAND_MAX*100-50));
            pt.setY(floor((float)rand() / RAND_MAX*100-50));
        }
        t.pts.enqueue(pt);
        while(t.pts.size() > TAILSIZE)
            t.pts.dequeue();
        i++;
    }
    repaint();
}
//------------------------------------------------------------------------------
