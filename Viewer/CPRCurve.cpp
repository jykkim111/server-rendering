#include "CPRCurve.h"
#include <qvector2d.h>

using namespace cgip;


CPRCurve::CPRCurve(QPixmap* pixmap) {

	//painter = new QPainter(m_pixmap);

	pen_forNormalBox = QPen(Qt::green, 1, Qt::SolidLine);
	pen_forNormalBox.setCosmetic(true);
	brush_forNormalBox = QBrush(QColor(20, 200, 20, 255));

	pen_forSelectedBox = QPen(Qt::red, 2, Qt::SolidLine);
	pen_forSelectedBox.setCosmetic(true);
	brush_forSelectedBox = QBrush(QColor(200, 50, 50, 255));

	pen_forPath = QPen(Qt::blue, 2, Qt::SolidLine);
	pen_forPath.setCosmetic(true);

	pen_forSelectedPath = QPen(Qt::red, 2, Qt::SolidLine);
	pen_forSelectedPath.setCosmetic(true);


	m_rightCircleForPath = new CPRCircle();
	m_rightCircleForPath->setRect(-3, -3, 6, 6);
	m_rightCircleForPath->setFlag(CPRCircle::ItemIsSelectable, true);
	
	m_leftCircleForPath = new CPRCircle();
	m_leftCircleForPath->setRect(-3, -3, 6, 6);
	m_leftCircleForPath->setFlag(CPRCircle::ItemIsSelectable, true);

	m_pathIndicator = new CPRLine();
	m_pathIndicator->setFlag(CPRLine::ItemIsSelectable, true);


	rightCircle_selected = false;
	leftCircle_selected = false; 

}


void CPRCurve::addPoint(QPointF point, bool isTemporary) {

	std::cout << "adding point : " << point.x() << ", " << point.y() << std::endl;

	controlPoints.push_back(point);
}

void CPRCurve::clearPoints() {
	controlPoints.clear();
	curvePoints.clear();
}

int CPRCurve::calculateNearestPoint(QPointF point) {
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

void CPRCurve::generatePathIndicator(std::vector<QPointF> vecPointList, int currentIndex, std::vector<QPointF>& pathList) {
	
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

	std::cout << "samplingRate : " << samplingRate << std::endl;


	//QVector2D point;
	QPointF point;
	for (int i = 0; i < 1024; i++) {
		float checking = samplingRate * i;
		point = QPointF(indicator_leftCirclePoint + m_currentPathNormal * samplingRate * i);
		pathList.push_back(point);
	}

	
}


void CPRCurve::generateCubicSpline(std::vector<QPointF> controlPointList, int nSampleNum, std::vector<QPointF>& vecPointList)
{
	int i, j;
	int nNumOfCtrlPt = controlPointList.size();
	///////////////////////////////////////////
	//	Natural Cubic Spline Matrix
	///////////////////////////////////////////
	double matEntry[3] = { 1 / 6.f, -2 / 6.f, 1 / 6.f };
	double matEntry2[3] = { 1 / 6.f, 4 / 6.f, 1 / 6.f };

	float** fMatrix = new float * [nNumOfCtrlPt + 2];
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
		px[i + 1] = (double)(controlPoints[i].x());
	}
	px[0] = 0;
	px[nNumOfCtrlPt + 1] = 0;

	for (i = 0; i < nNumOfCtrlPt; i++) {
		py[i + 1] = (double)(controlPoints[i].y());
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
	int nLineSeg = 1024;
	float fSamplingRate = 0.0f;

	QPointF temp;
	for (i = 0; i < nNumOfCtrlPt - 1; i++) {
		QPointF pntTemp = controlPoints[i] - controlPoints[i + 1];
		//nLineSeg = static_cast<int>(sqrt(pntTemp.x() * pntTemp.x() + pntTemp.y() * pntTemp.y()));
		if (nLineSeg < 1)
			nLineSeg = 1;
		
		fSamplingRate = static_cast<float>(1) / (nLineSeg / (nNumOfCtrlPt - 1));

		for (j = 0; j < static_cast<int> (nLineSeg / (nNumOfCtrlPt - 1)) + 1; j++) {
			temp.setX((bx[i] * pow(1 - j * fSamplingRate, 3) / 6 + bx[i + 1] * (3 * pow(j * fSamplingRate, 3) - 6 * pow(j * fSamplingRate, 2) + 4) / 6 +
				bx[i + 2] * (-3 * pow(j * fSamplingRate, 3) + 3 * pow(j * fSamplingRate, 2) + 3 * j * fSamplingRate + 1) / 6 +
				bx[i + 3] * pow(j * fSamplingRate, 3) / 6));
			temp.setY((by[i + 0] * pow(1 - j * fSamplingRate, 3) / 6 + by[i + 1] * (3 * pow(j * fSamplingRate, 3) - 6 * pow(j * fSamplingRate, 2) + 4) / 6 +
				by[i + 2] * (-3 * pow(j * fSamplingRate, 3) + 3 * pow(j * fSamplingRate, 2) + 3 * j * fSamplingRate + 1) / 6 +
				by[i + 3] * pow(j * fSamplingRate, 3) / 6));
			vecPointList.push_back(temp);
			//if (i == nNumOfCtrlPt - 2 && j == nLineSeg - 1) {
			//	temp.setX((bx[i + 0] * pow(1 - (j + 1) * fSamplingRate, 3) / 6 + bx[i + 1] * (3 * pow((j + 1) * fSamplingRate, 3) - 6 * pow((j + 1) * fSamplingRate, 2) + 4) / 6 +
			//		bx[i + 2] * (-3 * pow((j + 1) * fSamplingRate, 3) + 3 * pow((j + 1) * fSamplingRate, 2) + 3 * (j + 1) * fSamplingRate + 1) / 6 +
			//		bx[i + 3] * pow((j + 1) * fSamplingRate, 3) / 6));
			//	temp.setY((by[i + 0] * pow(1 - (j + 1) * fSamplingRate, 3) / 6 + by[i + 1] * (3 * pow((j + 1) * fSamplingRate, 3) - 6 * pow((j + 1) * fSamplingRate, 2) + 4) / 6 +
			//		by[i + 2] * (-3 * pow((j + 1) * fSamplingRate, 3) + 3 * pow((j + 1) * fSamplingRate, 2) + 3 * (j + 1) * fSamplingRate + 1) / 6 +
			//		by[i + 3] * pow((j + 1) * fSamplingRate, 3) / 6));
			//	vecPointList.push_back(temp);
			//}
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





void CPRCurve::drawPoints(QPainter* painter) {
	for (int i = 0; i < controlPoints.size(); i++) {
		QRectF new_control_point = QRectF(controlPoints[i].x() - 3, controlPoints[i].y() - 3, 6, 6);
		painter->setPen(pen_forNormalBox);
		painter->fillRect(new_control_point, brush_forNormalBox);
		painter->drawRect(new_control_point);
	}
}


void CPRCurve::drawCurve(QPainter *painter) {
	std::vector<QPointF> pathPointList;


	std::cout << "control point count: " << controlPoints.size() << std::endl;


	generateCubicSpline(controlPoints, 10, pathPointList);
	
	std::cout << "path point count: " << pathPointList.size() << std::endl;

	if (pathPointList.size() == 0)
		return;

	curvePoints = pathPointList;
	
	for (int j = pathPointList.size()-1; j > 0; j--) {
		QPointF p1 = pathPointList[j];
		QPointF p2 = pathPointList[j-1];
		painter->drawLine(p1, p2);
	}
	
	

}

void CPRCurve::drawPath(QPainter* painter) {
	
	std::vector<QPointF> pathList;
	
	if (m_curve_finished) {
		generatePathIndicator(curvePoints, m_currentPathIndex, pathList);
	}
	else {
		generatePathIndicator(curvePoints, 0, pathList);
	}

	if (pathList.size() == 0)
		return;

	pathPoints = pathList;
	
	std::cout << "pathPoints size: " << pathList.size() << std::endl;

	for (int j = pathList.size() - 1; j > 0; j--) {
		QPointF p1 = pathList[j];
		QPointF p2 = pathList[j - 1];
		painter->drawLine(p1, p2);
	}


	//painter->drawLine(indicator_leftCirclePoint, indicator_rightCirclePoint);
}


void CPRCurve::drawIndicatorCircle(QPainter* painter) {
	
	painter->drawEllipse(indicator_leftCirclePoint, 5, 5);
	painter->drawEllipse(indicator_rightCirclePoint, 5, 5);
}

void CPRCurve::changeCurrentCurveIndex(QPointF pos) {
	m_currentPathIndex = calculateNearestPoint(pos);
}