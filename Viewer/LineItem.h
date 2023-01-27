#pragma once

#include <qwidget.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsitem.h>
#include <qpoint.h>

class LineItem : public QObject, public QGraphicsLineItem
{

	Q_OBJECT
public: 
	LineItem(const QPointF, const QPointF);
	~LineItem(void);

	void activate(void);
	void deactivate(void);
	virtual QPainterPath shape() const override;



};

