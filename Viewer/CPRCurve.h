#pragma once

#include <qgraphicsscene.h>
#include <qobject.h>
#include <qaction.h>
#include <QLabel>
#include <QPainter>
#include <qmenu.h>
#include <glm/glm.hpp>

#include "../CgipCommon/Memory.h"
#include "../CgipCommon/CgipMatrix.h"
#include "../CgipCommon/CgipMatVector.h"


#include "CPRCircle.h"
#include "CPRLine.h"
#include "CPRPath.h"

class CPRCurve : public QObject
{
	Q_OBJECT

public:
	CPRCurve(QObject* parent = 0) : QObject(parent){}
	CPRCurve(QPixmap* pixmap);
	//~CPRCurve(void);

	void addPoint(QPointF point, bool isTemporary);
	void generateCubicSpline(std::vector<QPointF> controlPointList, int sample_num, std::vector<QPointF>& vecPointList);
	void generatePathIndicator(std::vector<QPointF> vecPointList, int currentIndex, std::vector<QPointF>& pathList);
	int calculateNearestPoint(QPointF point);
	//void generateCubicSpline(std::vector<float> x_series, std::vector<float> y_series, std::vector<QPointF> &path);
	/*
	void moveSelectedItem(QPointF point, bool isTemporary);

	inline int getPathIndex() { return 1; }
	inline int getPathLength() { return 1; }

	void deleteControlPoint();
	void finishCPRCurve(QPointF point);
	void generateCubicSpline(QList<QPointF> controlPoints, int sample_num, std::vector<QPointF> pointList);

	void clearSelection();
	void clear();
	std::vector<QPointF> getPointList();
	int nearestPointIndex(QPointF);
	void changeCPRPathIndex(int index);
	void showMenuBar(QPointF point);
	*/

	std::vector<QPointF> getCurvePoints() { return curvePoints; }
	std::vector<QPointF> getPathPoints() { return pathPoints; }

	int getNumOfControlPoints() { return controlPoints.size(); }

	QPointF getIndicatorLeftPoint() { return indicator_leftCirclePoint; }
	QPointF getIndicatorRightPoint() { return indicator_rightCirclePoint; }

	float getPathLength() { return m_path_length; }

	void drawPoints(QPainter* painter);
	void drawCurve(QPainter* painter);
	void drawPath(QPainter* painter);
	void drawIndicatorCircle(QPainter* painter);
	void start() { m_curve_finished = false; }
	void finish() { m_curve_finished = true; }
	bool is_finished() { return m_curve_finished; }

	void changeCurrentCurveIndex(QPointF);

	void clearPoints();

public slots:
	//void slotAddControlPoint();
	//void slotRemoveControlPoint();


private:

	//CPRPathIndicator* pathIndicator;
	//CPRCircle* rightCircleForPathIndicator;
	//CPRCircle* leftCircleForPathIndicator;


	//QPainter* painter;

	QPen pen_forNormalBox;
	QBrush brush_forNormalBox;

	QPen pen_forSelectedBox;
	QBrush brush_forSelectedBox;

	QPen pen_forPath;
	QPen pen_forSelectedPath;

	QMenu menu;
	QAction* actAddPoint;
	QAction* actRemovePoint;

	QPointF indicator_rightCirclePoint;
	QPointF indicator_leftCirclePoint;
	QPointF m_currentPathNormal;

	CPRLine* m_pathIndicator;
	CPRCircle* m_rightCircleForPath;
	CPRCircle* m_leftCircleForPath;
	CPRPath* m_curve;

	std::vector<QPointF> controlPoints;
	std::vector<QPointF> curvePoints;
	std::vector<QPointF> pathPoints;
	std::vector<QRectF> controlPointRects;

	bool rightCircle_selected;
	bool leftCircle_selected;
	bool pathIndicator_selected;
	bool path_initialized = false;

	bool m_curve_finished = false;
	int m_currentPathIndex = 0;
	int m_lengthOfIndicator = 80;

	float m_path_length;
};

