#pragma once

#include "EllipseItem.h"
#include <qpoint.h>

/*
	class EllipseItem_otf
	 : Re-implementation of EllipseItem.
	 : OTF polygon's points representation.
*/
class EllipseItem_otf : public EllipseItem
{
	Q_OBJECT
public:
	EllipseItem_otf(const QPointF point);
	~EllipseItem_otf(void);
	/**************************
		public interface.
	***************************/
	void activate(void);
	void deactivate(void);
	bool contains(const QPointF& point);
	


protected:

	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

};

