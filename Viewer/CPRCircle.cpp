#include "CPRCircle.h"
#include <QStyleOptionGraphicsItem>


CPRCircle::CPRCircle(QGraphicsItem* parent) : QGraphicsEllipseItem(parent)
{
	this->setFlag(CPRCircle::ItemIgnoresTransformations, true);

}

CPRCircle::~CPRCircle() {

}

void CPRCircle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget *widget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(option);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsEllipseItem::paint(painter, option, widget);
}