#include "OTF.h"
#include "OTFPolygon.h"
#include <qgraphicsscene.h>

OTFPolygon::OTFPolygon(void)
	: m_polygon(nullptr)
{
	m_polygon = new PolygonItem();
}

OTFPolygon::~OTFPolygon(void)
{
	//SAFE_DELETE_OBJECT(m_polygon);
	m_listPoint.clear();
	m_listLine.clear();
	m_listText.clear();
}


void OTFPolygon::addToScene(QGraphicsScene* scene)
{
	for (const auto& i : m_listPoint)	scene->addItem(i);
	for (const auto& i : m_listLine)	scene->addItem(i);
	for (const auto& i : m_listText)	scene->addItem(i);
	scene->addItem(m_polygon);
}

void OTFPolygon::removeFromScene(QGraphicsScene* scene)
{
	scene->removeItem(m_polygon);
	for (const auto& i : m_listPoint)	scene->removeItem(i);
	for (const auto& i : m_listLine)	scene->removeItem(i);
	for (const auto& i : m_listText)	scene->removeItem(i);
	this->clearUp();
}

void OTFPolygon::clearUp(void)
{
	//SAFE_DELETE_OBJECT(m_polygon);
	m_listPoint.clear();
	m_listLine.clear();
	m_listText.clear();

	m_viewPointList.clear();
	m_viewLineList.clear();
}

float OTFPolygon::mapCoord(int min, int max, int value) {
	float result = static_cast<float> ((value - min) * 400 / (max - min));
	return result;
}

void OTFPolygon::setMinMax(int min, int max) {
	min_intensity = min;
	max_intensity = max;
}

void OTFPolygon::setDefaultPolygon(const QPointF point, const int offset)
{
	clearUp();
	int position = static_cast<int>(point.x());

	// set points, lines, texts.
	QPolygon poly;
	QPoint pt1(position - 100, TF_HEIGHT - TF_MARGIN);
	QPoint pt2(position - 50, TF_MARGIN);
	QPoint pt3(position + 50, TF_MARGIN);
	QPoint pt4(position + 100, TF_HEIGHT - TF_MARGIN);

	poly.append(pt1);
	poly.append(pt2);
	poly.append(pt3);
	poly.append(pt4);


	m_polygon = new PolygonItem();
	m_polygon->setPolygon(poly);

	// add points.
	m_listPoint.append(new EllipseItem_otf(pt1));	//m_listPoint.at(0)->setIdx(0);
	m_listPoint.append(new EllipseItem_otf(pt2));	//m_listPoint.at(1)->setIdx(1);
	m_listPoint.append(new EllipseItem_otf(pt3));	//m_listPoint.at(2)->setIdx(2);
	m_listPoint.append(new EllipseItem_otf(pt4));	//m_listPoint.at(3)->setIdx(3);


	// add lines.
	m_listLine.append(new LineItem(pt2, pt3));

	// add texts.
	QFont font("Times", 10, QFont::Bold);
	QString str1, str2, str3, str4;

	m_listText.append(new TextItem(QPointF(pt1.x(), pt1.y() - 25), static_cast<float>(pt1.x() + offset)));
	m_listText.append(new TextItem(QPointF(pt2.x(), pt2.y() - 25), static_cast<float>(pt2.x() + offset)));
	m_listText.append(new TextItem(QPointF(pt3.x(), pt3.y() - 25), static_cast<float>(pt3.x() + offset)));
	m_listText.append(new TextItem(QPointF(pt4.x(), pt4.y() - 25), static_cast<float>(pt4.x() + offset)));
}

void OTFPolygon::setBonePresetPolygon(int offset)
{
	clearUp();
	// set points, lines, texts.
	QPolygon poly;
	QPoint pt1(130 - offset, TF_HEIGHT - TF_MARGIN);
	QPoint pt2(242 - offset, TF_MARGIN);
	QPoint pt3(1596 - offset, TF_MARGIN);
	QPoint pt4(1596 - offset, TF_HEIGHT-TF_MARGIN);


	// add points.
	m_listPoint.append(new EllipseItem_otf(pt1));	//m_listPoint.at(0)->setIdx(0);
	m_listPoint.append(new EllipseItem_otf(pt2));	//m_listPoint.at(1)->setIdx(1);
	m_listPoint.append(new EllipseItem_otf(pt3));	//m_listPoint.at(2)->setIdx(2);
	m_listPoint.append(new EllipseItem_otf(pt4));	//m_listPoint.at(3)->setIdx(3);

	poly.append(pt1);
	poly.append(pt2);
	poly.append(pt3);
	poly.append(pt4);

	m_polygon = new PolygonItem();
	m_polygon->setPolygon(poly);



	// add lines.
	m_listLine.append(new LineItem(pt2, pt3));

	// add texts.
	QFont font("Times", 10, QFont::Bold);
	QString str1, str2, str3, str4;

	m_listText.append(new TextItem(QPointF(pt1.x(), pt1.y() - 25), static_cast<float>(pt1.x() + offset)));
	m_listText.append(new TextItem(QPointF(pt2.x(), pt2.y() - 25), static_cast<float>(pt2.x() + offset)));
	m_listText.append(new TextItem(QPointF(pt3.x(), pt3.y() - 25), static_cast<float>(pt3.x() + offset)));
	m_listText.append(new TextItem(QPointF(pt4.x(), pt4.y() - 25), static_cast<float>(pt4.x() + offset)));
}


void OTFPolygon::setClothedPresetPolygon()
{
	clearUp();
	int offset = -1024;
	// set points, lines, texts.
	QPolygon poly;
	QPoint pt1(724, TF_HEIGHT - TF_MARGIN);
	QPoint pt2(989, TF_MARGIN);
	QPoint pt3(-1024, TF_MARGIN);
	QPoint pt4(1034, TF_HEIGHT - TF_MARGIN);


	// add points.
	m_listPoint.append(new EllipseItem_otf(pt1));	//m_listPoint.at(0)->setIdx(0);
	m_listPoint.append(new EllipseItem_otf(pt2));	//m_listPoint.at(1)->setIdx(1);
	m_listPoint.append(new EllipseItem_otf(pt3));	//m_listPoint.at(2)->setIdx(2);
	m_listPoint.append(new EllipseItem_otf(pt4));	//m_listPoint.at(3)->setIdx(3);

	poly.append(pt1);
	poly.append(pt2);
	poly.append(pt3);
	poly.append(pt4);

	m_polygon = new PolygonItem();
	m_polygon->setPolygon(poly);


	// add lines.
	m_listLine.append(new LineItem(pt2, pt3));


	// add texts.
	QFont font("Times", 10, QFont::Bold);
	QString str1, str2, str3, str4;

	m_listText.append(new TextItem(QPointF(pt1.x(), pt1.y() - 25), static_cast<float>(pt1.x()+ offset)));
	m_listText.append(new TextItem(QPointF(pt2.x(), pt2.y() - 25), static_cast<float>(pt2.x()+ offset)));
	m_listText.append(new TextItem(QPointF(pt3.x(), pt3.y() - 25), static_cast<float>(pt3.x()+ offset)));
	m_listText.append(new TextItem(QPointF(pt4.x(), pt4.y() - 25), static_cast<float>(pt4.x()+ offset)));
}





void OTFPolygon::movePolygon(float dx, int offset)
{
	QPolygonF tmp = m_polygon->polygon();
	tmp.translate(dx, 0);
	m_polygon->setPolygon(tmp);
	for (int i = 0; i < m_listPoint.size(); i++) {
		m_listPoint.at(i)->moveBy(dx, 0.0f);
		//m_viewPointList.at(i)->moveBy(dx, 0.0f);
		//m_listPoint.at(i)->moveBy(dx * (max_intensity - min_intensity) / 400, 0.0f);
	}
	for (const auto& i : m_listLine)
		i->setLine(i->line().x1() + dx, i->line().y1(), i->line().x2() + dx, i->line().y2());
	for (int i = 0; i < m_listText.size(); i++) {
		m_listText.at(i)->moveBy(dx, 0.0f);
		m_listText.at(i)->setVal(m_listPoint.at(i)->x());
		QString str;
		str.append("[" + QString().setNum(m_listText.at(i)->getVal() + offset, 'f', 0) + "]");
		m_listText.at(i)->setPlainText(str);
	}

}

void OTFPolygon::movePoint(int idx, float dx, float dy, int offset)
{
	// move specified point & update polygon.
	//m_viewPointList.at(idx)->moveBy(dx, dy);
	//m_listPoint.at(idx)->moveBy(dx * (max_intensity - min_intensity)/400, dy * -255/(TF_HEIGHT - 2* TF_MARGIN));
	m_listPoint.at(idx)->moveBy(dx, dy);
	QPolygonF tmp = m_polygon->polygon();
	QPointF point = tmp.at(idx);
	point.setX(m_listPoint.at(idx)->x());
	point.setY(m_listPoint.at(idx)->y());
	//point.setX(m_viewPointList.at(idx)->x());
	//point.setY(m_viewPointList.at(idx)->y());
	tmp.replace(idx, point);
	m_polygon->setPolygon(tmp);

	// move text.
	m_listText.at(idx)->moveBy(dx, dy);
	m_listText.at(idx)->setVal(m_listPoint.at(idx)->x());
	QString str;
	str.append("[" + QString().setNum(m_listText.at(idx)->getVal() + offset, 'f', 0) + "]");
	m_listText.at(idx)->setPlainText(str);

	// move line.
	if (m_listLine.empty())
		return;	// no line component.
	if (idx == 0 || idx == m_listPoint.size() - 1)
		return;	// no line editing.

	if (idx == 1) // one-end.
	{
		LineItem* tmp = m_listLine.at(0);
		tmp->setLine(m_listPoint.at(idx)->x(), m_listPoint.at(idx)->y(), tmp->line().x2(), tmp->line().y2());
	}
	else if (idx == m_listPoint.size() - 2) // one-end.
	{
		LineItem* tmp = m_listLine.at(idx - 2);
		tmp->setLine(tmp->line().x1(), tmp->line().y1(), m_listPoint.at(idx)->x(), m_listPoint.at(idx)->y());
	}
	else // inter-line's point.
	{
		LineItem* tmp1 = m_listLine.at(idx - 2);
		LineItem* tmp2 = m_listLine.at(idx - 1);
		tmp1->setLine(tmp1->line().x1(), tmp1->line().y1(), m_listPoint.at(idx)->x(), m_listPoint.at(idx)->y());
		tmp2->setLine(m_listPoint.at(idx)->x(), m_listPoint.at(idx)->y(), tmp2->line().x2(), tmp2->line().y2());
	}

}

void OTFPolygon::addPoint(const QPointF point, const int offset, QGraphicsScene* scene)
{
	QPointF newPoint;
	newPoint.setX(point.x());
	QPolygonF tmpPoly = m_polygon->polygon();

	// get new point's position. ( get point index of polygon)
	int idx = -1;
	if (point.x() < m_listPoint.at(0)->x())
		idx = 0;
	else if (point.x() > m_listPoint.last()->x())
		idx = m_listPoint.size();
	else {
		for (int i = 0; i < m_listPoint.size() - 1; i++) {
			if (point.x() >= m_listPoint.at(i)->x()
				&& point.x() < m_listPoint.at(i + 1)->x()) {
				idx = i + 1;
				break;
			}
		}
	}

	if (idx == -1) // failed.
		return;

	// get new point's position. (actual position at scene)
	if (idx == 0 || idx == m_listPoint.size())
		newPoint.setY(TF_HEIGHT - TF_MARGIN);
	else {
		newPoint.setY(
			(m_listPoint.at(idx)->y() - m_listPoint.at(idx - 1)->y()) *
			(newPoint.x() - m_listPoint.at(idx - 1)->x()) /
			(m_listPoint.at(idx)->x() - m_listPoint.at(idx - 1)->x()) +
			m_listPoint.at(idx - 1)->y()
		);
	}

	// add new point to polygon.
	tmpPoly.insert(idx, newPoint);
	m_polygon->setPolygon(tmpPoly);

	// add new point & other information, to lists.
	m_listPoint.insert(idx, new EllipseItem_otf(newPoint));
	scene->addItem(m_listPoint.at(idx));

	m_listText.insert(idx, new TextItem(QPointF(newPoint.x(), newPoint.y() - 25), static_cast<float>(newPoint.x() + offset)));
	scene->addItem(m_listText.at(idx));

	// remove & add line.
	if (idx == 1) { // add one line.
		m_listLine.insert(0, new LineItem(m_listPoint.at(1)->pos(), m_listPoint.at(2)->pos()));
		scene->addItem(m_listLine.at(0));
	}
	else if (idx == m_listPoint.size() - 2) { // add one line.
		m_listLine.push_back(new LineItem(m_listPoint.at(idx - 1)->pos(), m_listPoint.at(idx)->pos()));
		scene->addItem(m_listLine.last());
	}
	else { // remove one line & add two line.
		scene->removeItem(m_listLine.at(idx - 2));
		delete(m_listLine.at(idx - 2));
		m_listLine.replace(idx - 2, new LineItem(m_listPoint.at(idx - 1)->pos(), m_listPoint.at(idx)->pos()));
		m_listLine.insert(idx - 1, new LineItem(m_listPoint.at(idx)->pos(), m_listPoint.at(idx + 1)->pos()));
		scene->addItem(m_listLine.at(idx - 2));
		scene->addItem(m_listLine.at(idx - 1));
	}

	// set polygon color.
	QPen pen = m_polygon->pen();
	pen.setColor(Qt::red);
	m_polygon->setPen(pen);
}

void OTFPolygon::removePoint(const int pointIdx, QGraphicsScene* scene)
{
	// remove point of index(idx).
	if (pointIdx == -1)	return;
	QPolygonF tmpPoly = m_polygon->polygon();

	// remove from scene.
	scene->removeItem(m_listPoint.at(pointIdx));
	scene->removeItem(m_listText.at(pointIdx));

	// update if end-point.
	if (pointIdx == 0) {
		QPointF point(tmpPoly.at(pointIdx + 1));
		point.setY(TF_HEIGHT - TF_MARGIN);
		tmpPoly.replace(pointIdx + 1, point);
		m_listPoint.at(pointIdx + 1)->setY(TF_HEIGHT - TF_MARGIN);
		m_listText.at(pointIdx + 1)->setY(TF_HEIGHT - TF_MARGIN - 25);
	}
	else if (pointIdx == m_listPoint.size() - 1) {
		QPointF point(tmpPoly.at(pointIdx - 1));
		point.setY(TF_HEIGHT - TF_MARGIN);
		tmpPoly.replace(pointIdx - 1, point);
		m_listPoint.at(pointIdx - 1)->setY(TF_HEIGHT - TF_MARGIN);
		m_listText.at(pointIdx - 1)->setY(TF_HEIGHT - TF_MARGIN - 25);
	}

	// remove and modify line.
	if (pointIdx == 0 || pointIdx == 1) { // left-end points.
		// remove line.
		scene->removeItem(m_listLine.at(0));
		delete(m_listLine.at(0));
		m_listLine.removeAt(0);
	}
	else if (pointIdx == m_listPoint.size() - 1 || pointIdx == m_listPoint.size() - 2) { // right-end points.
		// remove line.
		scene->removeItem(m_listLine.last());
		delete(m_listLine.last());
		m_listLine.removeLast();
	}
	else {
		// remove 1, modify 1. (or remove 2, add 1)
		scene->removeItem(m_listLine.at(pointIdx - 1));
		delete(m_listLine.at(pointIdx - 1));
		m_listLine.removeAt(pointIdx - 1);
		m_listLine.at(pointIdx - 2)->setLine(m_listLine.at(pointIdx - 2)->line().x1(), m_listLine.at(pointIdx - 2)->line().y1(), m_listPoint.at(pointIdx + 1)->pos().x(), m_listPoint.at(pointIdx + 1)->pos().y());
	}

	// update.
	// remove from TFPOLYGON.
	tmpPoly.removeAt(pointIdx);
	m_listPoint.removeAt(pointIdx);
	m_listText.removeAt(pointIdx);
	m_polygon->setPolygon(tmpPoly);
}

void OTFPolygon::moveLine(int idx, float dx, float dy, int offset)
{
	int ptIdx1 = idx + 1;
	int ptIdx2 = idx + 2;

	this->movePoint(ptIdx1, dx, dy, offset);
	this->movePoint(ptIdx2, dx, dy, offset);
	
	
}

void OTFPolygon::deactivateAll(void)
{
	m_polygon->deactivate();
	for (const auto& i : m_listPoint)	i->deactivate();
	for (const auto& i : m_listLine)	i->deactivate();
	//for (const auto& i : m_viewPointList)	i->deactivate();
	//for (const auto& i : m_viewLineList)	i->deactivate();
}

