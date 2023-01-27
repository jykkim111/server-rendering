#pragma once
#include "../ViewerManager/global.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <qgraphicsitem.h>
#include <QGraphicsScene>
#include <QWidget>


class MPR_GuideLine : public QObject
{
	Q_OBJECT
public:
	MPR_GuideLine(SliceType slice_type);

	//void setVisible(bool visible);
	//void setThicknessVisible(bool visible);	
	//void setPos(float fx, float fy);
	void rotate(const float angle);
	void addLineToScene(QGraphicsScene* pScene);
	//void initGuideLine(const QPointF ptCenter);
	void setCenter(QPointF ptCenter);
	QPointF getCenter() { return m_center_point; }
	//void setTransformOriginPoint(QPointF pt);
	void setLine(const QPointF, const float, const float, const float);
	void setLineVisible(bool);
	void setCenterEllipseVisible(bool);
	void setHorizontalLineWidth(const float width);
	void setVerticalLineWidth(const float width);

	void setHorizontalThickness(float thickness);
	void setVerticalThickness(float thickness);


private:
	QGraphicsLineItem* m_guidelines_hL;
	QGraphicsLineItem* m_guidelines_hR;
	QGraphicsLineItem* m_guidelines_vT;
	QGraphicsLineItem* m_guidelines_vB;
	QGraphicsLineItem* m_thicknessLines_hL;
	QGraphicsLineItem* m_thicknessLines_hR;
	QGraphicsLineItem* m_thicknessLines_vT;
	QGraphicsLineItem* m_thicknessLines_vB;
	QGraphicsEllipseItem* m_center_ellipse_pt;
	QGraphicsEllipseItem* m_center_ellipse;

	int radius;
	float m_angle;
	float m_margin_translate;

	QPointF m_center_point;

};

