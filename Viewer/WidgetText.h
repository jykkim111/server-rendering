#pragma once
#include <qwidget.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsitem.h>
class WidgetText : public QGraphicsTextItem
{
	Q_OBJECT
public:
	WidgetText(QWidget* parent = 0);
	~WidgetText(void);

signals:
	void sigPressed(void);

protected:
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

};

