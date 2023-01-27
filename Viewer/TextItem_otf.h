#pragma once
#include <qwidget.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsitem.h>

class TextItem_otf : public QGraphicsTextItem
{
	Q_OBJECT
public:
	TextItem_otf(QWidget* parent = 0);
	~TextItem_otf(void);

signals:
	void sigPressed(void);

protected:
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

};

