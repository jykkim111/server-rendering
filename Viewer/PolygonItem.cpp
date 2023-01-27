#include "PolygonItem.h"
#include <qpen.h>
#include <iostream>

PolygonItem::PolygonItem(void)
{
	// set UI color components.
	QPen temppen(Qt::white);	
	temppen.setCosmetic(true);
	QBrush brush(Qt::green);
	QColor color(Qt::green);	
	color.setAlphaF(0.0);
	brush.setColor(color);

	this->setAcceptHoverEvents(true);
	this->setZValue(1.0f);
	this->setBrush(brush);

	// set polygon color.
	QPen pen = this->pen();
	pen.setColor(Qt::red);
	this->setPen(pen);

	QObject::installEventFilter(this);
}

PolygonItem::~PolygonItem(void)
{
}

void PolygonItem::activate(void)
{
	QBrush brush = this->brush();
	QColor color(Qt::green);
	color.setAlphaF(0.3);
	brush.setColor(color);
	this->setBrush(brush);
	QPen pen = this->pen();
	pen.setColor(Qt::yellow);
	this->setPen(pen);
}

void PolygonItem::deactivate(void)
{
	QBrush brush = this->brush();
	QColor color(Qt::red);
	color.setAlphaF(0.1);
	brush.setColor(color);
	this->setBrush(brush);

	QPen pen = this->pen();
	pen.setColor(Qt::red);
	pen.setWidth(1);
	this->setPen(pen);
}



void PolygonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsPolygonItem::hoverLeaveEvent(event);
	QBrush brush = this->brush();
	QColor color(Qt::red);
	color.setAlphaF(0.1);
	brush.setColor(color);
	this->setBrush(brush);

	QPen pen = this->pen();
	pen.setColor(Qt::red);
	pen.setWidth(1);
	this->setPen(pen);
}
