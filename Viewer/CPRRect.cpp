#include "CPRRect.h"
#include <QStyleOptionGraphicsItem>


CPRRect::CPRRect(QGraphicsItem* parent) : QGraphicsRectItem(parent)
{
	this->setFlag(CPRRect::ItemIgnoresTransformations, true);
	this->setFlag(CPRRect::ItemIsSelectable, true);
	this->setZValue(200);

}

CPRRect::~CPRRect() {

}

void CPRRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(option);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsRectItem::paint(painter, option, widget);
}

