#include "CPRPath.h"
#include <QStyleOptionGraphicsItem>


CPRPath::CPRPath(QGraphicsItem* parent) : QGraphicsPathItem(parent)
{
	//this->setFlag(CPRPath::ItemIgnoresTransformations, true);

}

CPRPath::~CPRPath() {

}

void CPRPath::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(option);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsPathItem::paint(painter, option, widget);
}


void CPRPath::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {


	//QMenu menu;
	//QAction* addAction = menu.addAction("Add control Point");
	//QAction* selectedAction = menu.exec(QCursor::pos());
}

/*
void CPRPath::mousePressEvent(QGraphicsSceneMouseEvent event) {
	if (event.button() == Qt::LeftButton) {
		this->setSelected(true);
	}
}

void CPRPath::mouseReleaseEvent(QGraphicsSceneMouseEvent event) {
	if (event.button() == Qt::LeftButton) {
		this->setSelected(false);
	}
}
*/