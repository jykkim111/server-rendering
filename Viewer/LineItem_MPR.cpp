#include "LineItem_MPR.h"
#include <iostream>

 LineItem_MPR::LineItem_MPR(QGraphicsItem* parent) : QGraphicsLineItem(parent) {
	 setAcceptHoverEvents(true);
	 setAcceptedMouseButtons(Qt::LeftButton);
}

 LineItem_MPR::~LineItem_MPR() {

 }

 void LineItem_MPR::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
	 std::cout << "currently hovering..." << std::endl;
	 QGraphicsLineItem::hoverEnterEvent(event);
 }

 void LineItem_MPR::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
 }

 void LineItem_MPR::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
 {
	 if (option->state & QStyle::State_MouseOver) {
		 std::cout << "current hovering... " << std::endl;
		 this->pen().setWidth(5.0f);
	 }
	 else {
		 this->pen().setWidth(2.0f);
	 }
 }