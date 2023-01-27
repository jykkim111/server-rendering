#include "EllipseItem_color.h"
#include <qpen.h>

EllipseItem_color::EllipseItem_color(const QPointF point)
	: EllipseItem(point),
	m_iIdx(-1)
{
	QPen pen(Qt::white);		pen.setCosmetic(true);
	QColor color(Qt::blue);
	QBrush brush(Qt::blue);	//brush.setColor(color);

	this->setAcceptHoverEvents(true);
	this->setAcceptedMouseButtons(Qt::MouseButton::LeftButton);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setZValue(5.0f);
	this->setPen(pen);
	this->setBrush(brush);
	this->setPos(point);
	this->setRect(-10, -5, 20, 10);
}

EllipseItem_color::~EllipseItem_color(void)
{
}

void EllipseItem_color::activate(void)
{
	this->setRect(-12, -10, 24, 20);
}


void EllipseItem_color::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsEllipseItem::hoverLeaveEvent(event);
	this->setRect(-10, -5, 20, 10);
}


