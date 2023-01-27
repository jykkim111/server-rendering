#pragma once

#include <qobject.h>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <qmenu.h>
#include <QCursor>

class CPRPath : public QObject, public QGraphicsPathItem
{
	Q_OBJECT

public:
	CPRPath(QGraphicsItem* parent = 0);
	~CPRPath();

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	//void mousePressEvent(QGraphicsSceneMouseEvent event);
	//void mouseReleaseEvent(QGraphicsSceneMouseEvent event);
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

