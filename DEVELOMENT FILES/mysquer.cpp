#include "mysquer.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

MySquer::MySquer(block ne, bool hole) {
    name_squ = QString::fromStdString(ne.name);
    size = ne.size;
    start = ne.start;
    Pressed = hole;
}

QRectF MySquer::boundingRect() const {
    return QRectF(0, start, 100, size);
}

void MySquer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QRectF rec = boundingRect();
    QBrush brush(Qt::blue);
    if (!Pressed) {
        brush.setColor(Qt::red);
    } else {
        brush.setColor(Qt::green);
    }
    painter->fillRect(rec, brush);
    painter->drawRect(rec);
}