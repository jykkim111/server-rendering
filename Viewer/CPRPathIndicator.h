#pragma once

#include <qobject.h>
#include <QGraphicsLineItem>


class CPRPathIndicator : public QObject, public QGraphicsLineItem
{
	Q_OBJECT

public: 
	CPRPathIndicator(QGraphicsItem* parent = 0);
	//~CPRPathIndicator();

	void setSelectedWidth(float width);
	void setZoomFactor(float zoom);
	//virtual QPainterPath shape() const override;

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
	float m_width;
	float m_zoom;

};

