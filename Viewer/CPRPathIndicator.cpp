#include "CPRPathIndicator.h"
#include <QStyleOptionGraphicsItem>

CPRPathIndicator::CPRPathIndicator(QGraphicsItem* parent) : QGraphicsLineItem(parent) {
	this->setFlag(CPRPathIndicator::ItemIsSelectable, true);
	this->setZValue(200);
	m_width = 5.0f;
	m_zoom = 1.0f;
}


void CPRPathIndicator::setSelectedWidth(float width) {
	m_width = width / m_zoom;
	if (m_width < 1)
		m_width = 1;

	if (m_width > 5)
		m_width = 5;

}

void CPRPathIndicator::setZoomFactor(float zoom) {
	m_zoom = zoom;
	m_width = m_width / m_zoom;

	if (m_width < 1)
		m_width = 1;

	if (m_width > 5)
		m_width = 5;
}

void CPRPathIndicator::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

	QStyleOptionGraphicsItem* style
		= const_cast<QStyleOptionGraphicsItem*>(option);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsLineItem::paint(painter, option, widget);
}

