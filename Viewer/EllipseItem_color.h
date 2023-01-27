#pragma once

#include "EllipseItem.h"
#include <qpoint.h>

/*
	* OTFColor structure.
	* represents color control point.
*/
struct OTFColor {
	int	_intensity;	// pixel density.
	QColor	_color;	// color value according to the density.
};


/*
	class CAzEllipseItem_color
	 : Re-implementation of CAzEllipseItem.
	 : OTF color's points representation.
*/
class EllipseItem_color : public EllipseItem
{
	Q_OBJECT
public:
	EllipseItem_color(const QPointF point);
	~EllipseItem_color(void);
	/**************************
		public interface.
	***************************/
	inline void setIdx(const int idx) { m_iIdx = idx; }
	inline void setColor(const OTFColor& color) { m_color = color; }

	inline int getIdx(void) { return m_iIdx; }
	inline OTFColor	getColor(void) { return m_color; }

	void activate(void);

protected:
	// overriding functions.
	//virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	//virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

	//QPainterPath shape(void) const;

signals:
	// signals of interactions.
	void sigPressed(int);

private:
	// private member fields.
	int		m_iIdx;		// Index of color object.
	OTFColor	m_color;	// color value.
};

