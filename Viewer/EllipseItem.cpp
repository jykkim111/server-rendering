#include "EllipseItem.h"
#include <qpen.h>


EllipseItem::EllipseItem(const QPointF point) {
	QPen pen(Qt::white);
	pen.setCosmetic(true);
	QColor color(Qt::green);
	color.setAlpha(0.0f);
	QBrush brush(Qt::green);
	brush.setColor(color);

	this->setAcceptHoverEvents(true);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setZValue(5.0f);
	this->setPen(pen);
	this->setBrush(brush);
	this->setPos(point);
}

EllipseItem::~EllipseItem(void) {

}

void EllipseItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
	QGraphicsEllipseItem::hoverEnterEvent(event);
}

void EllipseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
	QGraphicsEllipseItem::hoverLeaveEvent(event);
}

void EllipseItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	QGraphicsEllipseItem::mousePressEvent(event);
}