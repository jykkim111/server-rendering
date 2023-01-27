#include "TextItem_otf.h"

TextItem_otf::TextItem_otf(QWidget* parent) {
	setDefaultTextColor(Qt::white);
	setAcceptHoverEvents(true);

	QFont font("Times", 12);
	setFont(font);

	setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

}

TextItem_otf::~TextItem_otf(void) {
	
}


void TextItem_otf::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{

}

void TextItem_otf::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsTextItem::hoverEnterEvent(event);
	QFont font("Times", 12);
	font.setBold(true);
	setFont(font);
}

void TextItem_otf::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsTextItem::hoverLeaveEvent(event);
	QFont font("Times", 12);
	font.setBold(false);
	setFont(font);
}

void TextItem_otf::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsTextItem::mousePressEvent(event);
	emit sigPressed();
}