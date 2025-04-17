#ifndef MYSQUER_H
#define MYSQUER_H

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QString>
#include "memory_manager.h"

class MySquer : public QGraphicsItem {
public:
    QString name_squ;
    int start, size;
    MySquer();
    MySquer(block ne, bool hole);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    bool Pressed;
};

#endif // MYSQUER_H