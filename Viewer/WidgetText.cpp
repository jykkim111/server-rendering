#include "WidgetText.h"

WidgetText::WidgetText(QWidget* parent)
{
	setDefaultTextColor(Qt::white);
	setAcceptHoverEvents(true);

	QFont font("Times", 12);
	setFont(font);

	setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

WidgetText::~WidgetText(void)
{
}

void WidgetText::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{

}

void WidgetText::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsTextItem::hoverEnterEvent(event);
	QFont font("Times", 12);
	font.setBold(true);
	setFont(font);
}

void WidgetText::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsTextItem::hoverLeaveEvent(event);
	QFont font("Times", 12);
	font.setBold(false);
	setFont(font);
}

void WidgetText::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsTextItem::mousePressEvent(event);
	emit sigPressed();
}