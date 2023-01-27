#pragma once

#include <qobject.h>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>


class CPRRect : public QObject, public QGraphicsRectItem
{
	Q_OBJECT

public:
	CPRRect(QGraphicsItem* parent = 0);
	~CPRRect();

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
};
