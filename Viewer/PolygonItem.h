#pragma once

#include "EllipseItem_otf.h"
#include <qgraphicsitem.h>
#include <qpoint.h>

class PolygonItem : public QObject, public QGraphicsPolygonItem
{
	Q_OBJECT
public:
	PolygonItem(void);
	~PolygonItem(void);
	/**************************
		public interface.
	***************************/
	void activate(void);
	void deactivate(void);
protected:
	
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

};

