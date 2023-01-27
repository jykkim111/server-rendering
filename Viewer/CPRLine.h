#pragma once

#include <qobject.h>
#include <QGraphicsLineItem>


class CPRLine : public QObject, public QGraphicsLineItem
{
	Q_OBJECT

public:
	CPRLine(QGraphicsItem* parent = 0);
	~CPRLine();

	void setSelectedWidth(float width);
	void setZoomFactor(float zoom_factor);
	virtual QPainterPath shape() const override;

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
	float m_width;
	float m_zoom_factor;
};


