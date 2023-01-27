#include "GraphicsCurve.h"
#include <qvector2d.h>

using namespace cgip;


GraphicsCurve::GraphicsCurve(QGraphicsScene* scene) {

	//painter = new QPainter(m_pixmap);

	m_scene = scene;

	pen_forNormalBox = QPen(Qt::green, 1, Qt::SolidLine);
	pen_forNormalBox.setCosmetic(true);
	brush_forNormalBox = QBrush(QColor(20, 200, 20, 255));

	pen_forSelectedBox = QPen(Qt::red, 2, Qt::SolidLine);
	pen_forSelectedBox.setCosmetic(true);
	brush_forSelectedBox = QBrush(QColor(200, 50, 50, 255));

	pen_forCircle = QPen(Qt::yellow, 2, Qt::SolidLine);
	pen_forCircle.setCosmetic(true);
	brush_forCircle = QBrush(Qt::yellow);
	
	pen_forIndicator = QPen(Qt::green, 2, Qt::SolidLine);
	pen_forIndicator.setCosmetic(true);

	pen_forPath = QPen(Qt::blue, 2, Qt::SolidLine);
	pen_forPath.setCosmetic(true);


	pen_forSelectedPath = QPen(Qt::red, 2, Qt::SolidLine);
pen_forSelectedPath.setCosmetic(true);


m_rightCircleForPath = new CPRCircle();
//m_rightCircleForPath->setRect(-3, -3, 6, 6);
m_rightCircleForPath->setFlag(CPRCircle::ItemIsSelectable, true);

m_leftCircleForPath = new CPRCircle();
//m_leftCircleForPath->setRect(-3, -3, 6, 6);
m_leftCircleForPath->setFlag(CPRCircle::ItemIsSelectable, true);

m_pathIndicator = new CPRPath();
m_pathIndicator->setFlag(CPRPath::ItemIsSelectable, true);

m_curve = new CPRPath();
m_curve->setFlag(CPRPath::ItemIsSelectable, true);


bool rightCircleSelected = false;
bool leftCircleSelected = false;
bool pathGuideSelected = false;
bool curveSelected = false;

m_action_addPoint = new QAction("Add control point", this);
connect(m_action_addPoint, SIGNAL(triggered()), this, SLOT(slotAddControlPoint()));

m_action_removePoint = new QAction("Remove control point", this);
connect(m_action_removePoint, SIGNAL(triggered()), this, SLOT(slotRemoveControlPoint()));

m_action_addPathBar = new QAction("Add Path bar", this);
connect(m_action_addPathBar, SIGNAL(triggered()), this, SLOT(slotAddPathBar()));

m_menu.addAction(m_action_addPoint);
m_menu.addAction(m_action_removePoint);
m_menu.addAction(m_action_addPathBar);

m_menu.hide();
}

GraphicsCurve::~GraphicsCurve() {

	for (int i = 0; i < controlPointRects.size(); i++) {
		m_scene->removeItem(controlPointRects[i]);
	}
	m_scene->removeItem(m_curve);
	SAFE_DELETE_OBJECT(m_curve);

	m_scene->removeItem(m_leftCircleForPath);
	SAFE_DELETE_OBJECT(m_leftCircleForPath);

	m_scene->removeItem(m_rightCircleForPath);
	SAFE_DELETE_OBJECT(m_rightCircleForPath);

	m_scene->removeItem(m_pathIndicator);
	SAFE_DELETE_OBJECT(m_pathIndicator);

	controlPoints.clear();
	curvePoints.clear();

}

void GraphicsCurve::showMenuBar(QPointF point) {
	m_pointMenu = point;
	m_menu.exec(QCursor::pos());
}


void GraphicsCurve::addPoint(QPointF point, bool isTemporary) {

	//std::cout << "adding point : " << point.x() << ", " << point.y() << std::endl;

	controlPoints.push_back(QPointF(point.x(), point.y()));

	CPRRect* pointRect = new CPRRect();
	pointRect->setRect(-3, -3, 6, 6);
	pointRect->setPen(pen_forNormalBox);
	pointRect->setBrush(brush_forNormalBox);
	//pointRect->setPos(point.x(), point.y());
	m_scene->addItem(pointRect);
	pointRect->setPos(point.x(), point.y());

	controlPointRects.push_back(pointRect);
}

void GraphicsCurve::addScreenPoint(QPointF point) {
	controlPoints_screen.push_back(QPointF(point.x(), point.y()));
}


void GraphicsCurve::slotAddControlPoint() {
	int indexOfNewPoint = calculateNearestPoint(m_pointMenu);
	QPointF newPoint = curvePoints[indexOfNewPoint];

	CPRRect* newRect = new CPRRect();
	newRect->setRect(-3, -3, 6, 6);
	newRect->setPen(pen_forNormalBox);
	newRect->setBrush(brush_forNormalBox);
	m_scene->addItem(newRect);
	newRect->setPos(newPoint.x(), newPoint.y());

	int k = 0;
	float e = 0.01f;
	for (int i = 0; i <= indexOfNewPoint; i++) {
		if (controlPoints[k].x() <= (curvePoints[i].x() + e) && controlPoints[k].x() >= (curvePoints[i].x() - e)
			&& controlPoints[k].y() <= (curvePoints[i].y() + e) && controlPoints[k].y() >= (curvePoints[i].y() - e)){

			k++;
		}
	}
	controlPointRects.insert(controlPointRects.begin() + k, newRect);
	controlPoints.insert(controlPoints.begin() + k, newPoint);
	addedNewPoints = true;
}

void GraphicsCurve::slotRemoveControlPoint() {

	if (controlPoints.size() == 2) {
		return;
	}
	else {
		controlPoints.erase(controlPoints.begin() + m_selectedPointIndex);

		CPRRect *rectToRemove = controlPointRects[m_selectedPointIndex];
		m_scene->removeItem(rectToRemove);
		SAFE_DELETE_OBJECT(rectToRemove);
		controlPointRects.erase(controlPointRects.begin() + m_selectedPointIndex);

		drawCurve();
		drawNewPathGuide();

		removedExistingPoints = true;
	}
}


int GraphicsCurve::calculateNearestPoint(QPointF point) {
	int temp = 0;
	float min_val = 1000000;
	float minTemp = 0;

	QPointF diff;
	for (int i = 0; i < curvePoints.size(); i++) {
		diff = point - curvePoints[i];
		minTemp = diff.x() * diff.x() + diff.y() * diff.y();
		if (minTemp < min_val) {
			min_val = minTemp;
			temp = i;
		}
	}

	return temp;
}

void GraphicsCurve::generatePathIndicator(std::vector<QPointF> vecPointList, int currentIndex, std::vector<QPointF>& pathList) {

	QPointF diff;
	if (currentIndex != vecPointList.size() - 1) {
		diff = vecPointList[currentIndex + 1] - vecPointList[currentIndex];
	}
	else {
		diff = vecPointList[currentIndex] - vecPointList[currentIndex - 1];
	}

	diff /= sqrt(diff.x() * diff.x() + diff.y() * diff.y());
	m_currentPathNormal.setX(diff.y());
	m_currentPathNormal.setY(-diff.x());

	indicator_rightCirclePoint = QPointF(vecPointList[currentIndex] + m_currentPathNormal * m_lengthOfIndicator);
	indicator_leftCirclePoint = QPointF(vecPointList[currentIndex] - m_currentPathNormal * m_lengthOfIndicator);
	QVector2D leftCircle = QVector2D(indicator_leftCirclePoint);
	QVector2D rightCircle = QVector2D(indicator_rightCirclePoint);
	QVector2D path_length = rightCircle - leftCircle;
	m_path_length = sqrt(path_length.x() * path_length.x() + path_length.y() * path_length.y());
	float samplingRate = m_path_length / 1024;



	//QVector2D point;
	QPointF point;
	for (int i = 0; i < 1024; i++) {
		float checking = samplingRate * i;
		point = QPointF(indicator_leftCirclePoint + m_currentPathNormal * samplingRate * i);
		pathList.push_back(point);
	}


}


void GraphicsCurve::generateCubicSpline(std::vector<QPointF> controlPointList, int nSampleNum, std::vector<QPointF>& vecPointList)
{
	int i, j;
	int nNumOfCtrlPt = controlPointList.size();
	///////////////////////////////////////////
	//	Natural Cubic Spline Matrix
	///////////////////////////////////////////
	double matEntry[3] = { 1 / 6.f, -2 / 6.f, 1 / 6.f };
	double matEntry2[3] = { 1 / 6.f, 4 / 6.f, 1 / 6.f };

	float** fMatrix = new float* [nNumOfCtrlPt + 2];
	for (i = 0; i < nNumOfCtrlPt + 2; i++) {
		fMatrix[i] = new float[nNumOfCtrlPt + 2];
		memset(fMatrix[i], 0, sizeof(float) * (nNumOfCtrlPt + 2));
	}

	for (i = 0; i < nNumOfCtrlPt + 2; i++) {
		for (j = 0; j < nNumOfCtrlPt + 2; j++) {
			if (i == 0) {
				if (j - 3 < 0)
					fMatrix[i][j] = matEntry[j];
			}
			else if (i == nNumOfCtrlPt + 1) {
				if (nNumOfCtrlPt - j - 2 < 0)
					fMatrix[i][j] = matEntry[j - nNumOfCtrlPt + 1];
			}
			else {
				if (j >= i - 1 && j <= i - 1 + 2)
					fMatrix[i][j] = matEntry2[j - i + 1];
			}
		}
	}

	///////////////////////////////////////////
	// Solve Linear System
	///////////////////////////////////////////
	CgipMatrix A;//matrixN A;
	A.SetSize(nNumOfCtrlPt + 2, nNumOfCtrlPt + 2);
	for (i = 0; i < nNumOfCtrlPt + 2; i++) {
		for (j = 0; j < nNumOfCtrlPt + 2; j++) {
			A.SetValue(i, j, fMatrix[i][j]);
		}
	}

	double* px = new double[nNumOfCtrlPt + 2];
	double* bx = new double[nNumOfCtrlPt + 2];
	double* py = new double[nNumOfCtrlPt + 2];
	double* by = new double[nNumOfCtrlPt + 2];


	for (i = 0; i < nNumOfCtrlPt; i++) {
		px[i + 1] = (double)(controlPointList[i].x());
	}
	px[0] = 0;
	px[nNumOfCtrlPt + 1] = 0;

	for (i = 0; i < nNumOfCtrlPt; i++) {
		py[i + 1] = (double)(controlPointList[i].y());
	}
	py[0] = 0;
	py[nNumOfCtrlPt + 1] = 0;


	CgipMatVector pxVec;
	pxVec.SetSize(nNumOfCtrlPt + 2);
	pxVec.SetValue(px);

	CgipMatVector pyVec;
	pyVec.SetSize(nNumOfCtrlPt + 2);
	pyVec.SetValue(py);

	CgipMatVector bxVec;
	bxVec.SetSize(nNumOfCtrlPt + 2);
	bxVec.Solve(A, pxVec);
	for (i = 0; i < nNumOfCtrlPt + 2; i++) {
		bx[i] = bxVec.GetValue(i);
	}

	CgipMatVector byVec;
	byVec.SetSize(nNumOfCtrlPt + 2);
	byVec.Solve(A, pyVec);
	for (i = 0; i < nNumOfCtrlPt + 2; i++) {
		by[i] = byVec.GetValue(i);
	}

	///////////////////////////////////////////
	//	Generate 3D Curve Point
	///////////////////////////////////////////
	//int nLineSeg = nSampleNum;
	//float fSamplingRate = static_cast<float>(1)/nLineSeg;
	int nLineSeg = 0;
	float fSamplingRate = 0.0f;

	QPointF temp;
	for (i = 0; i < nNumOfCtrlPt - 1; i++) {
		QPointF pntTemp = controlPointList[i] - controlPointList[i + 1];
		//nLineSeg = static_cast<int>(sqrt(pntTemp.x() * pntTemp.x() + pntTemp.y() * pntTemp.y()));
		//std::cout << "nLineSeg : " << nLineSeg << std::endl;
		nLineSeg = 1024;
		if (nLineSeg < 1)
			nLineSeg = 1;

		fSamplingRate = static_cast<float>(1) / nLineSeg;

		for (j = 0; j < nLineSeg; j++) {
			temp.setX((bx[i] * pow(1 - j * fSamplingRate, 3) / 6 + bx[i + 1] * (3 * pow(j * fSamplingRate, 3) - 6 * pow(j * fSamplingRate, 2) + 4) / 6 +
				bx[i + 2] * (-3 * pow(j * fSamplingRate, 3) + 3 * pow(j * fSamplingRate, 2) + 3 * j * fSamplingRate + 1) / 6 +
				bx[i + 3] * pow(j * fSamplingRate, 3) / 6));
			temp.setY((by[i + 0] * pow(1 - j * fSamplingRate, 3) / 6 + by[i + 1] * (3 * pow(j * fSamplingRate, 3) - 6 * pow(j * fSamplingRate, 2) + 4) / 6 +
				by[i + 2] * (-3 * pow(j * fSamplingRate, 3) + 3 * pow(j * fSamplingRate, 2) + 3 * j * fSamplingRate + 1) / 6 +
				by[i + 3] * pow(j * fSamplingRate, 3) / 6));
			vecPointList.push_back(temp);
			if (i == nNumOfCtrlPt - 2 && j == nLineSeg - 1) {
				temp.setX((bx[i + 0] * pow(1 - (j + 1) * fSamplingRate, 3) / 6 + bx[i + 1] * (3 * pow((j + 1) * fSamplingRate, 3) - 6 * pow((j + 1) * fSamplingRate, 2) + 4) / 6 +
					bx[i + 2] * (-3 * pow((j + 1) * fSamplingRate, 3) + 3 * pow((j + 1) * fSamplingRate, 2) + 3 * (j + 1) * fSamplingRate + 1) / 6 +
					bx[i + 3] * pow((j + 1) * fSamplingRate, 3) / 6));
				temp.setY((by[i + 0] * pow(1 - (j + 1) * fSamplingRate, 3) / 6 + by[i + 1] * (3 * pow((j + 1) * fSamplingRate, 3) - 6 * pow((j + 1) * fSamplingRate, 2) + 4) / 6 +
					by[i + 2] * (-3 * pow((j + 1) * fSamplingRate, 3) + 3 * pow((j + 1) * fSamplingRate, 2) + 3 * (j + 1) * fSamplingRate + 1) / 6 +
					by[i + 3] * pow((j + 1) * fSamplingRate, 3) / 6));
				vecPointList.push_back(temp);
			}
		}
	}

	for (i = 0; i < nNumOfCtrlPt + 2; i++) {
		SAFE_DELETE_ARRAY(fMatrix[i]);
	}
	SAFE_DELETE_ARRAY(fMatrix);
	SAFE_DELETE_ARRAY(px);
	SAFE_DELETE_ARRAY(bx);
	SAFE_DELETE_ARRAY(py);
	SAFE_DELETE_ARRAY(by);
}





void GraphicsCurve::drawCurve() {
	std::vector<QPointF> pathPointList;
	std::vector<QPointF> screenPointList_screen;

	generateCubicSpline(controlPoints, 10, pathPointList);


	if (pathPointList.size() == 0)
		return;

	curvePoints = pathPointList;

	QPainterPath painter(pathPointList[pathPointList.size() - 1]);
	m_curve->setPen(pen_forPath);

	for (int j = pathPointList.size() - 1; j > 0; j--) {
		painter.lineTo(pathPointList[j].x(), pathPointList[j].y());
	}

	if (m_curve_finished) {
		m_curve->setPath(painter);
	}
	else {
		m_curve->setPath(painter);
		m_scene->addItem(m_curve);

	}
}

void GraphicsCurve::slotAddPathBar() {
	int indexOfNewPoint = calculateNearestPoint(m_pointMenu);
	m_currentPathIndex = indexOfNewPoint;
	std::vector<QPointF> pathList;
	generatePathIndicator(curvePoints, m_currentPathIndex, pathList);

	if (pathList.size() == 0)
		return;

	pathPoints = pathList;


	m_leftCircleForPath->setPen(pen_forCircle);
	m_leftCircleForPath->setBrush(brush_forCircle);
	m_leftCircleForPath->setRect(-3, -3, 6, 6);

	//m_leftCircleForPath->setRect(pathList[0].x() - 3, pathList[0].y() - 3, 6, 6);

	m_leftCircleForPath->setZValue(100);
	m_rightCircleForPath->setPen(pen_forCircle);
	m_rightCircleForPath->setBrush(brush_forCircle);
	m_rightCircleForPath->setRect(-3, -3, 6, 6);

	//m_rightCircleForPath->setRect(pathList[pathList.size() - 1].x() - 3, pathList[pathList.size() - 1].y() - 3, 6, 6);

	m_rightCircleForPath->setZValue(100);
	m_pathIndicator->setPen(pen_forIndicator);

	m_scene->addItem(m_leftCircleForPath);
	m_scene->addItem(m_rightCircleForPath);

	m_leftCircleForPath->setPos(pathList[0].x(), pathList[0].y());
	m_rightCircleForPath->setPos(pathList[pathList.size() - 1].x(), pathList[pathList.size() - 1].y());

	QPainterPath guidePainter(pathList[pathList.size() - 1]);
	m_pathIndicator->setPen(pen_forIndicator);

	for (int j = pathList.size() - 1; j > 0; j--) {
		guidePainter.lineTo(pathList[j].x(), pathList[j].y());
	}

	m_pathIndicator->setPath(guidePainter);
	m_scene->addItem(m_pathIndicator);

	emit sigPathBarAdded();

}

void GraphicsCurve::drawNewPathGuide() {
	std::vector<QPointF> pathList;
	std::vector<QPointF> pathList_screen;
	generatePathIndicator(curvePoints, m_currentPathIndex, pathList);

	if (pathList.size() == 0)
		return;

	pathPoints = pathList;
	pathPoints_screen = pathList_screen;


	m_leftCircleForPath->setPen(pen_forCircle);
	m_leftCircleForPath->setBrush(brush_forCircle);
	m_leftCircleForPath->setRect(-3, -3, 6, 6);

	//m_leftCircleForPath->setRect(pathList[0].x() - 3, pathList[0].y() - 3, 6, 6);

	m_leftCircleForPath->setZValue(100);
	m_rightCircleForPath->setPen(pen_forCircle);
	m_rightCircleForPath->setBrush(brush_forCircle);
	m_rightCircleForPath->setRect(-3, -3, 6, 6);

	//m_rightCircleForPath->setRect(pathList[pathList.size() - 1].x() - 3, pathList[pathList.size() - 1].y() - 3, 6, 6);

	m_rightCircleForPath->setZValue(100);
	m_pathIndicator->setPen(pen_forIndicator);

	m_leftCircleForPath->setPos(pathList[0].x(), pathList[0].y());
	m_rightCircleForPath->setPos(pathList[pathList.size() - 1].x(), pathList[pathList.size() - 1].y());



	QPainterPath guidePainter(pathList[pathList.size() - 1]);
	m_pathIndicator->setPen(pen_forIndicator);

	for (int j = pathList.size() - 1; j > 0; j--) {
		guidePainter.lineTo(pathList[j].x(), pathList[j].y());
	}

	m_pathIndicator->setPath(guidePainter);

}



void GraphicsCurve::moveSelectedItem(QPointF point, bool temp) {
	if (temp) {
		m_previousPoint = QPointF(point.x(), point.y());
	}
	else {
		if (pathGuideSelected) {

			m_currentPathIndex = calculateNearestPoint(point);
			drawNewPathGuide();
		}

		else if (curveSelected) {

			QPointF pntDiff = m_previousPoint - QPointF(point.x(), point.y());

			for (int i = 0; i < controlPoints.size(); i++) {
				controlPointRects[i]->setRect(-3, -3, 6, 6);
				controlPointRects[i]->setPos(controlPoints[i].x() - pntDiff.x(), controlPoints[i].y() - pntDiff.y());

				controlPoints[i].setX(controlPoints[i].x() - pntDiff.x());
				controlPoints[i].setY(controlPoints[i].y() - pntDiff.y());
			}

			drawCurve();

			drawNewPathGuide();

			m_previousPoint = point;
		}

		else if (controlPointSelected) {

			controlPoints[m_selectedPointIndex].setX(point.x());
			controlPoints[m_selectedPointIndex].setY(point.y());
			controlPointRects[m_selectedPointIndex]->setRect(-3, -3, 6, 6);
			controlPointRects[m_selectedPointIndex]->setPos(point.x(), point.y());


			drawCurve();
			drawNewPathGuide();
		}

		else if (rightCircleSelected) {

			QPointF diff = point - curvePoints[m_currentPathIndex];
			int temp = 0;
			temp = static_cast<int> (m_currentPathNormal.x() * diff.x() + m_currentPathNormal.y() * diff.y());
			if (temp < 10) {
				return;
			}
			m_lengthOfIndicator = temp;
			drawNewPathGuide();
		}

		else if (leftCircleSelected) {

			QPointF diff = curvePoints[m_currentPathIndex] - point;
			int temp = 0;
			temp = static_cast<int> (m_currentPathNormal.x() * diff.x() + m_currentPathNormal.y() * diff.y());
			if (temp < 10) {
				return;
			}
			m_lengthOfIndicator = temp;
			drawNewPathGuide();

		}
	}

	

}


bool GraphicsCurve::hasSelectedControlPoint() {

	controlPointSelected = false;

	for (int i = 0; i < controlPointRects.size(); i++) {
		bool point_is_selected = controlPointRects[i]->isSelected();

		if (point_is_selected) {
			controlPointRects[i]->setPen(pen_forSelectedBox);
			controlPointRects[i]->setBrush(brush_forSelectedBox);
			m_curve->setPen(pen_forSelectedPath);
			m_selectedPointIndex = i;
			controlPointSelected = true;
		}
	}

	return controlPointSelected;

}

bool GraphicsCurve::hasSelectedPathGuide() {

	if (m_rightCircleForPath->isSelected()) {
		rightCircleSelected = true;
		return true;
	}

	if (m_leftCircleForPath->isSelected()) {
		leftCircleSelected = true;
		return true;
	}

	if (m_pathIndicator->isSelected()) {
		pathGuideSelected = true;
		return true;
	}

	return false;

}

bool GraphicsCurve::hasSelectedCurve() {

	hasSelectedItem = false;

	if (m_curve->isSelected()) {
		curveSelected = true;
		hasSelectedItem = true;
		m_curve->setPen(pen_forSelectedPath);
		printf("selected curve\n");
	}

	return hasSelectedItem;
}

void GraphicsCurve::unselectAll() {
	for (int i = 0; i < controlPointRects.size(); i++) {
		controlPointRects[i]->setBrush(brush_forNormalBox);
		controlPointRects[i]->setPen(pen_forNormalBox);
	}

	m_selectedPointIndex = -1;
	leftCircleSelected = false;
	rightCircleSelected = false;
	pathGuideSelected = false;
	curveSelected = false;

	m_curve->setPen(pen_forPath);
}



void GraphicsCurve::changeCurrentCurveIndex(QPointF pos) {
	m_currentPathIndex = calculateNearestPoint(pos);
}

