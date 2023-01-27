#pragma once
#include <qwidget.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsitem.h>
#include <qpoint.h>

class EllipseItem : public QObject, public QGraphicsEllipseItem
{
	Q_OBJECT;
public:
	EllipseItem(const QPointF point);
	~EllipseItem(void);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};

