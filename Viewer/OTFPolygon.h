#pragma once
#include "PolygonItem.h"
#include "EllipseItem_otf.h"
#include "LineItem.h"
#include "TextItem.h"
#include <qobject.h>
#include <qpoint.h>

/*
	class OTFPolygon
	 : represents polygon / points / lines / texts.
	 : provide interface for this polygon interactions.
*/
class OTFPolygon : public QObject
{
	Q_OBJECT
public:
	OTFPolygon(void);
	~OTFPolygon(void);
	/**************************
		public interface.
	***************************/
	void addToScene(QGraphicsScene* scene);
	void removeFromScene(QGraphicsScene* scene);

	void setDefaultPolygon(const QPointF point, const int offset);
	void setBonePresetPolygon(int offset);
	void setClothedPresetPolygon();
	void setMinMax(int min, int max);
	void movePolygon(float dx, int offset);
	void movePoint(int idx, float dx, float dy, int offset);
	void moveLine(int idx, float dx, float dy, int offset);

	void addPoint(const QPointF point, const int offset, QGraphicsScene* scene);
	void removePoint(const int pointIdx, QGraphicsScene* scene);

	float mapCoord(int min, int max, int value);
	void deactivateAll(void);

	inline PolygonItem* getPolygon(void) { return m_polygon; }
	inline QList<EllipseItem_otf*>& getPointList(void) { return m_listPoint; }
	inline QList<EllipseItem_otf*>& getViewPointList(void) { return m_listPoint; }
	inline QList<LineItem*>& getLineList(void) { return m_listLine; }
	inline QList<LineItem*>& getViewLineList(void) { return m_listLine;  }
	inline QList<TextItem*>& getTextList(void) { return m_listText; }


private:
	// private functions.
	void clearUp(void);
private:
	// private member fields.

	PolygonItem* m_polygon;
	QList<EllipseItem_otf*>		m_listPoint;
	QList<EllipseItem_otf*>		m_viewPointList;
	QList<LineItem*>		m_listLine;
	QList<LineItem*>		m_viewLineList;
	QList<TextItem*>		m_listText;

	int	min_intensity = -1024;
	int max_intensity = 1596;
};

