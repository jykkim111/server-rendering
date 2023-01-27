#include "EllipseItem_otf.h"
#include <qpen.h>
#include <iostream>

EllipseItem_otf::EllipseItem_otf(const QPointF point)
	: EllipseItem(point)
	//m_iPolyIdx(-1),
	//m_iIdx(-1),
	//m_point(point)
{
	QPen pen(Qt::white);		pen.setCosmetic(true);
	QColor color(Qt::blue);
	QBrush brush(Qt::blue);	//brush.setColor(color);

	this->setAcceptHoverEvents(true);
	this->setAcceptedMouseButtons(Qt::MouseButton::LeftButton);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setZValue(10.0f);
	this->setParentItem(nullptr);
	this->setPen(pen);
	this->setBrush(brush);
	this->setPos(point);
	this->setRect(-5, -5, 10, 10);
}

EllipseItem_otf::~EllipseItem_otf(void)
{
}

void EllipseItem_otf::activate(void)
{
	this->setRect(-10, -10, 20, 20);
}

bool EllipseItem_otf::contains(const QPointF& point)
{
	// check if near the point.
	if (std::abs(this->pos().x() - point.x()) < 15
		&& std::abs(this->pos().y() - point.y()) < 15)
	{
		//setDefaultGeometry();
		this->activate();
		return true;
	}
	return false;
}


void EllipseItem_otf::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsEllipseItem::hoverLeaveEvent(event);
	this->setRect(-5, -5, 10, 10);
}

void EllipseItem_otf::deactivate(void)
{
	this->setRect(-5, -5, 10, 10);
}