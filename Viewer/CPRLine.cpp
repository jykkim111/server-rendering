#include "CPRLine.h"
#include <QStyleOptionGraphicsItem>


CPRLine::CPRLine(QGraphicsItem* pParent) :
	QGraphicsLineItem(pParent)
{
	//this->setFlag(CPRLine::ItemIgnoresTransformations, false);
	this->setFlag(CPRLine::ItemIsSelectable, true);
	this->setZValue(200);
	m_width = 5.0f;
	m_zoom_factor = 1.0f;
}

CPRLine::~CPRLine()
{

}

void CPRLine::setSelectedWidth(float width)
{
	m_width = width / m_zoom_factor;
	if (m_width < 1)
		m_width = 1;

	if (m_width > 5)
		m_width = 5;
}

void CPRLine::setZoomFactor(float zoom)
{
	m_zoom_factor = zoom;
	m_width = m_width / m_zoom_factor;
	if (m_width < 1)
		m_width = 1;

	if (m_width > 5)
		m_width = 5;
}

void CPRLine::paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget)
{
	QStyleOptionGraphicsItem* style
		= const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsLineItem::paint(pPainter, pOption, pWidget);
}

QPainterPath CPRLine::shape() const
{
	QPainterPath path(QPointF(this->line().x1(), this->line().y1()));
	path.lineTo(QPointF(this->line().x2(), this->line().y2()));
	QPainterPathStroker stroker;
	stroker.setWidth(m_width);
	stroker.setJoinStyle(Qt::PenJoinStyle::BevelJoin);
	return stroker.createStroke(path);
}