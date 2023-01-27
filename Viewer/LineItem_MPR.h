#pragma once
#include <qgraphicsitem.h>
#include <qobject.h>
#include <QGraphicsLineItem>
#include <QPen>
#include <qstyleoption.h>
#include <qgraphicssceneevent.h>
#include <QPainter>

class LineItem_MPR : public QObject, QGraphicsLineItem
{
	Q_OBJECT

public:
	LineItem_MPR(QGraphicsItem* parent = 0);
	~LineItem_MPR();

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
};

