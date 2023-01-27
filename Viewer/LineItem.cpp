#include "LineItem.h"
#include <qpen.h>

LineItem::LineItem(const QPointF pt1, const QPointF pt2)
	: QGraphicsLineItem(pt1.x(), pt1.y(), pt2.x(), pt2.y())
{
	this->setLine(pt1.x(), pt1.y(), pt2.x(), pt2.y());

	QPen pen(Qt::green);		pen.setCosmetic(true);
	pen.setWidth(3);

	this->setAcceptedMouseButtons(Qt::MouseButton::LeftButton);
	//this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setZValue(5.0f);
	this->setPen(pen);

	this->setVisible(true);
}

LineItem::~LineItem(void)
{
}

void LineItem::activate(void)
{
	this->setVisible(true);
}

void LineItem::deactivate(void)
{
	this->setVisible(false);
}

QPainterPath LineItem::shape() const
{
	QPainterPath path(QPointF(this->line().x1(), this->line().y1()));
	path.lineTo(QPointF(this->line().x2(), this->line().y2()));
	QPainterPathStroker stroker;
	stroker.setWidth(10.0f);
	stroker.setJoinStyle(Qt::PenJoinStyle::BevelJoin);
	return stroker.createStroke(path);
}