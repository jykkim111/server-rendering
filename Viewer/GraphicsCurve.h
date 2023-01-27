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
#include "CPRRect.h"

class GraphicsCurve : public QObject
{
	Q_OBJECT

public:
	GraphicsCurve(QObject* parent = 0) : QObject(parent) {}
	GraphicsCurve(QGraphicsScene* scene);
	~GraphicsCurve(void);

	void addPoint(QPointF point, bool isTemporary);
	void addScreenPoint(QPointF point);
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

	CPRPath getCurve() { return m_curve; }
	CPRPath getPathIndicator() { return m_pathIndicator; }

	QPointF getIndicatorLeftPoint() { return indicator_leftCirclePoint; }
	QPointF getIndicatorRightPoint() { return indicator_rightCirclePoint; }

	float getPathLength() { return m_path_length; }
	int getNumOfControlPoints() { return controlPoints.size(); }

	void drawCurve();
	void drawNewPathGuide();
	bool hasSelectedControlPoint();
	bool hasSelectedPathGuide();
	bool hasSelectedCurve();
	void unselectAll();
	void moveSelectedItem(QPointF, bool);


	void start() { m_curve_finished = false; }
	void finish() { m_curve_finished = true; }
	bool is_finished() { return m_curve_finished; }

	void changeCurrentCurveIndex(QPointF);
	

	void completeAddingNewPoint() { addedNewPoints = false; }
	void completeRemovingExistingPoint() { removedExistingPoints = false; }
	bool AddedNewPoint() { return addedNewPoints; }
	bool RemovedExistingPoint() { return removedExistingPoints; }



	void showMenuBar(QPointF point);

	//void clearPoints();


signals:
	void sigPathBarAdded();


public slots:
	void slotAddControlPoint();
	void slotRemoveControlPoint();
	void slotAddPathBar();


private:

	//CPRPathIndicator* pathIndicator;
	//CPRCircle* rightCircleForPathIndicator;
	//CPRCircle* leftCircleForPathIndicator;


	//QPainter* painter;

	QGraphicsScene* m_scene = nullptr;

	QPen pen_forNormalBox;
	QBrush brush_forNormalBox;

	QPen pen_forSelectedBox;
	QBrush brush_forSelectedBox;

	QPen pen_forPath;
	QPen pen_forSelectedPath;

	QPen pen_forCircle;
	QBrush brush_forCircle;

	QPen pen_forIndicator;


	QMenu menu;
	QAction* actAddPoint;
	QAction* actRemovePoint;

	QPointF indicator_rightCirclePoint;
	QPointF indicator_leftCirclePoint;
	QPointF m_currentPathNormal;

	CPRPath* m_pathIndicator;
	CPRCircle* m_rightCircleForPath;
	CPRCircle* m_leftCircleForPath;
	CPRPath* m_curve;

	std::vector<QPointF> controlPoints;
	std::vector<QPointF>controlPoints_screen;
	std::vector<QPointF> curvePoints;
	std::vector<QPointF> curvePoints_screen;
	std::vector<QPointF> pathPoints;
	std::vector<QPointF> pathPoints_screen;
	std::vector<CPRRect*> controlPointRects;

	bool rightCircleSelected;
	bool leftCircleSelected;
	bool pathGuideSelected;
	bool curveSelected;
	bool controlPointSelected;

	QPointF m_previousPoint;

	bool hasSelectedItem = false;

	bool path_initialized = false;
	bool curve_initialized = false;

	bool m_curve_finished = false;
	int m_currentPathIndex = 0;
	int m_selectedPathIndex = -1;
	int m_selectedPointIndex = -1;
	int m_lengthOfIndicator = 80;

	float m_path_length;


	bool addedNewPoints = false;
	bool removedExistingPoints = false;


	QMenu m_menu;

	QAction* m_action_addPoint;
	QAction* m_action_removePoint;
	QAction* m_action_addPathBar;

	QPointF m_pointMenu;


};

