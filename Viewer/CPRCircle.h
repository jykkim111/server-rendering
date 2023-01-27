#pragma once

#include <qobject.h>
#include <QGraphicsEllipseItem>


class CPRCircle : public QObject, public QGraphicsEllipseItem
{
	Q_OBJECT

public:
	CPRCircle(QGraphicsItem* parent = 0);
	~CPRCircle();

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

};

