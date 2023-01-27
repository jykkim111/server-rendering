#include "OTFWidget.h"
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QMenu>
#include <QColorDialog>
#include <qfiledialog.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qsettings.h>
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <direct.h>
#include <string>

#include <qdir.h>

OTFWidget::OTFWidget(QWidget* parent) :
	QGraphicsScene(parent)
{
	m_currElem._which = COMPONENT::NONE;
	m_currElem._iIdx = -1;
	m_currElem._iPolyIdx = -1;
	m_histogram = nullptr;
	m_mainColorRect = nullptr;

	m_currLine = addLine(0, 0, 0, 100, QPen(Qt::white));
	QFont font("Times", 10, QFont::Bold);
	QString temp;
	temp.append("  [" + QString().setNum(0.0, 'f', 0) + "]");
	m_currText = addText(temp, font);
	m_currText->setDefaultTextColor(Qt::white);
	m_currText->setPos(0, 60);
	m_currText->setVisible(false);
	m_currText->setFlag(QGraphicsRectItem::ItemIgnoresTransformations, true);
	m_currText->setZValue(5.0f);

	m_iTFSize = 0;
	m_iOffset = 0;
	m_isPressed = false;
	m_isOnPolygon = false;
	m_bTextPressed = false;

	// Export & Preset.
	m_pTextPreset = new WidgetText();
	m_pTextPreset->setDefaultTextColor(Qt::green);
	m_pTextPreset->setPlainText("Preset");
	m_pTextPreset->setPos(0, 0);
	m_pTextPreset->setZValue(5.0f);
	this->addItem(m_pTextPreset);

	m_pTextExport = new WidgetText();
	m_pTextExport->setDefaultTextColor(Qt::green);
	m_pTextExport->setPlainText("Export");
	m_pTextExport->setPos(0, 20);
	m_pTextExport->setZValue(5.0f);
	this->addItem(m_pTextExport);

	connect(m_pTextPreset, SIGNAL(sigPressed(void)), this, SLOT(slotLoadPreset(void)));
	connect(m_pTextExport, SIGNAL(sigPressed(void)), this, SLOT(slotExport(void)));

	m_menu = new QMenu();
	m_addPolygonAct = new QAction(tr("Add Polygon"), this);
	m_removePolygonAct = new QAction(tr("Remove Polygon"), this);
	m_addPointAct = new QAction(tr("Add Control Point"), this);
	m_removePointAct = new QAction(tr("Remove Control Point"), this);
	m_addColorObjectAct = new QAction(tr("Add New Color"), this);
	m_updateColorObjectAct = new QAction(tr("Modify color"), this);
	m_removeColorObjectAct = new QAction(tr("Remove color"), this);

	connect(m_addPolygonAct, SIGNAL(triggered()), this, SLOT(slotAddPolygon()));
	connect(m_removePolygonAct, SIGNAL(triggered()), this, SLOT(slotRemovePolygon()));
	connect(m_addPointAct, SIGNAL(triggered()), this, SLOT(slotAddPoint()));
	connect(m_removePointAct, SIGNAL(triggered()), this, SLOT(slotRemovePoint()));
	connect(m_addColorObjectAct, SIGNAL(triggered()), this, SLOT(slotAddColor()));
	connect(m_updateColorObjectAct, SIGNAL(triggered()), this, SLOT(slotUpdateColor()));
	connect(m_removeColorObjectAct, SIGNAL(triggered()), this, SLOT(slotRemoveColor()));
}

OTFWidget::~OTFWidget()
{
	m_listOTFObj.clear();
	//SAFE_DELETE_OBJECT(m_histogram);
	//SAFE_DELETE_OBJECT(m_currLine);
	//SAFE_DELETE_OBJECT(m_currText);
	//SAFE_DELETE_OBJECT(m_mainColorRect);
	//SAFE_DELETE_OBJECT(m_pTf);
}

void OTFWidget::slotViewPortChanged(float x)
{
	m_pTextExport->setPos(x, m_pTextExport->pos().y());
	m_pTextPreset->setPos(x, m_pTextPreset->pos().y());
}


/* draw Histogram & initialize */
void OTFWidget::initOTF(int min, int max, int offset)
{
	
	//m_min = min;
	//m_max = max;

	m_min = -2048;
	m_max = 4000;

	
	m_iTFSize = max - min;
	//m_iOffset = offset;
	m_iOffset = -2048;
	this->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
	this->setSceneRect(0, 0, m_iTFSize, TF_HEIGHT);

	QPen pen;						pen.setColor(Qt::gray);
	QPen mainPen(Qt::transparent);	mainPen.setWidth(0);
	//if (m_histogram)
	//	removeItem(m_histogram);
	//SAFE_DELETE_OBJECT(m_histogram);
	//m_histogram = addPolygon(QPolygon(), QPen(Qt::gray), QBrush(Qt::gray));
	//SAFE_DELETE_OBJECT(m_mainColorRect);
	m_mainColorRect = addRect(0, TF_HEIGHT - TF_MARGIN + 10, m_iTFSize, TF_MARGIN / 2 + 2, mainPen, QBrush(Qt::white));
	m_mainColorRect->setZValue(0.0f);

	// find max value
	//int iMax = 0;
	//for (int i = 0; i < m_iTFSize; i++) {
	//	if (histogram[i] > iMax)
	//		iMax = histogram[i];
	//}
	////float fMax = std::logf(static_cast<float>(iMax));
	//// draw histogram
	//QPolygon histo;
	//histo.append(QPoint(-1, TF_HEIGHT - TF_MARGIN));
	//if (iMax != 0) {
	//	for (int i = 0; i < m_iTFSize; i++) {
	//		float temp;
	//		if (histogram[i] != 0)
	//			temp = static_cast<float>(histogram[i]) / iMax;
	//		else
	//			temp = 0.0f;
	//		histo.append(QPoint(i, TF_HEIGHT - TF_MARGIN - (TF_HEIGHT - 2 * TF_MARGIN) * temp));
	//	}
	//}
	//else {
	//	for (int i = 0; i < m_iTFSize; i++)
	//		histo.append(QPoint(i, TF_HEIGHT - TF_MARGIN));
	//}
	//histo.append(QPoint(m_iTFSize + 1, TF_HEIGHT - TF_MARGIN));
	//histo.append(QPoint(-1, TF_HEIGHT - TF_MARGIN));
	//
	//m_histogram->setPolygon(histo);
	//
	// DELETE EXISTING RESOURCES.
	for (int i = 0; i < m_listOTFObj.size(); i++) {
		OTFPolygon* temp = m_listOTFObj.at(i);
		temp->removeFromScene(this);
		delete (m_listOTFObj.at(i));
		m_listOTFObj.replace(i, nullptr);
	}
	m_listOTFObj.clear();
	m_colorList.clear();
	for (int i = 0; i < m_colorObjectList.size(); i++)
		this->removeItem(m_colorObjectList.at(i));
	m_colorObjectList.clear();

	// Read presets & set default preset.
	//m_bDental = size > 4096 ? true : false;
	initLoadPresets();
	//loadBone();
}

/*
	Read all presets.
	 - load all .bmp files,
	 - save all .tf file paths
*/
void OTFWidget::initLoadPresets(void) {
	m_listTFPaths.clear();
	QDir directory("./presets");
	//directory.setCurrent(":/../../SubProjects/Engine/Module/AzDVRMod/Resources/TF_presets");

	if (!directory.exists())
		std::cout << "no directory." << std::endl;
	QFileInfoList listInfo = directory.entryInfoList(QDir::Files, QDir::Name);
	for (const auto& i : listInfo) {
		m_listTFPaths.push_back(i.filePath());
	}
	for (const auto& i : m_listTFPaths) {
		if (i.contains("default")) {
			loadPresets(i.toStdString());
			break;
		}
	}
}
void OTFWidget::slotExport(void)
{
	m_bTextPressed = true;

	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TF files(*.tf)"));
	if (fileName.size() == 0) // no file name typed. (canceled dialog)
		return;

	std::ofstream outFile;
	outFile.open(fileName.toStdString(), std::ios::out);
	if (!outFile.is_open())
		return;

	// WRITE.
	outFile << m_listOTFObj.size() << " " << m_colorList.size() << "\n";
	for (int i = 0; i < m_listOTFObj.size(); i++) {
		OTFPolygon* tmp = m_listOTFObj.at(i);
		outFile << tmp->getPointList().size() << "\n";
		for (int j = 0; j < tmp->getPointList().size(); j++) {
			if (m_iOffset < 0) {
				outFile << tmp->getPointList().at(j)->pos().x() - m_iOffset << " " << tmp->getPointList().at(j)->pos().y() << "\n";
			}
			else {
				outFile << tmp->getPointList().at(j)->pos().x() + m_iOffset << " " << tmp->getPointList().at(j)->pos().y() << "\n";
			}
		}
	}
	for (int i = 0; i < m_colorList.size(); i++) {
		OTFColor tmp = m_colorList.at(i);
		if (m_iOffset < 0) {
			outFile << tmp._intensity - m_iOffset << " " << tmp._color.red() << " " << tmp._color.green() << " " << tmp._color.blue() << "\n";
		}
		else {
			outFile << tmp._intensity + m_iOffset << " " << tmp._color.red() << " " << tmp._color.green() << " " << tmp._color.blue() << "\n";
		}
		
	}
	outFile.close();
	clearSelection();
	//emit sigExport(fileName);

	m_listTFPaths.clear();

	QDir directory("./presets");
	//directory.setCurrent(":/../../SubProjects/Engine/Module/AzDVRMod/Resources/TF_presets");

	if (!directory.exists())
		std::cout << "no directory." << std::endl;
	QFileInfoList listInfo = directory.entryInfoList(QDir::Files, QDir::Name);
	for (const auto& i : listInfo) {
		m_listTFPaths.push_back(i.filePath());
	}
}

void OTFWidget::slotLoadPreset(void){
		m_bTextPressed = true;
		emit sigOTFPresetLoad();
}

void OTFWidget::slotSelected(int idx)
{
	this->loadPresets(m_listTFPaths.at(idx).toStdString());
}

void OTFWidget::loadPresets(const std::string& path)
{
	std::ifstream inFile;
	inFile.open(path, std::ios::in);
	if (!inFile.is_open())
		return;
	
	// DELETE EXISTING RESOURCES.
	for (int i = 0; i < m_listOTFObj.size(); i++) {
		OTFPolygon* temp = m_listOTFObj.at(i);
		temp->removeFromScene(this);
		delete (m_listOTFObj.at(i));
		m_listOTFObj.replace(i, nullptr);
	}
	m_listOTFObj.clear();
	m_colorList.clear();
	for (int i = 0; i < m_colorObjectList.size(); i++)
		this->removeItem(m_colorObjectList.at(i));
	m_colorObjectList.clear();

	// READ.
	int polyNum;
	int colorNum;
	inFile >> polyNum;
	inFile >> colorNum;

	// set UI color components.
	QPen temppen(Qt::white);	temppen.setCosmetic(true);
	QBrush brush(Qt::green);
	QColor color(Qt::green);	color.setAlphaF(0.0);
	brush.setColor(color);
	QFont font("Times", 10, QFont::Bold);

	for (int i = 0; i < polyNum; i++) // READ POLYGON
	{
		OTFPolygon* tmpObj = new OTFPolygon();
		//tmpObj->setMinMax(m_min, m_max);

		// read # of points.
		int numPts;
		inFile >> numPts;

		QPolygon poly;
		for (int j = 0; j < numPts; j++) {
			float x, y;
			inFile >> x >> y;
			
			if (m_iOffset < 0) {
				QPoint pt(x - m_iOffset, y);
				poly.append(pt);
				tmpObj->getPointList().append(new EllipseItem_otf(pt));
				QString str("[" + QString().setNum(static_cast<float>(pt.x()) - m_iOffset, 'f', 0) + "]");
				tmpObj->getTextList().append(new TextItem(QPointF(pt.x(), pt.y() - 25), static_cast<float>(pt.x() + m_iOffset)));
			}
			else {
				QPoint pt(x + m_iOffset, y);
				poly.append(pt);
				tmpObj->getPointList().append(new EllipseItem_otf(pt));
				QString str("[" + QString().setNum(static_cast<float>(pt.x()) + m_iOffset, 'f', 0) + "]");
				tmpObj->getTextList().append(new TextItem(QPointF(pt.x(), pt.y() - 25), static_cast<float>(pt.x() - m_iOffset)));
			}
			
		}
		tmpObj->getPolygon()->setPolygon(poly);
		tmpObj->getPolygon()->setPen(QPen(Qt::transparent));
		tmpObj->getPolygon()->setBrush(brush);
		QPen pen = tmpObj->getPolygon()->pen();
		pen.setColor(Qt::red);
		tmpObj->getPolygon()->setPen(pen);

		// add lines.
		QList<EllipseItem_otf*>& pointList = tmpObj->getPointList();
		QList<LineItem*>& lineList = tmpObj->getLineList();
		for (int i = 1; i < pointList.size() - 2; i++)
			lineList.append(new LineItem(pointList.at(i)->pos(), pointList.at(i + 1)->pos()));

		// ADD.
		tmpObj->addToScene(this);

		// update to m_listOTFObj <CAzOTFPolygon>
		m_listOTFObj.append(tmpObj);
	}

	for (int i = 0; i < colorNum; i++) // READ COLOR.
	{
		int posX, R, G, B;
		inFile >> posX >> R >> G >> B;

		QColor C(R, G, B);
		
		QGraphicsEllipseItem* temp;
		QPen pen;
		pen.setColor(Qt::gray);
		if (C.red() > 125 && C.green() > 125 && C.blue() > 125) {
			pen.setColor(Qt::black);
		}
		temp = addEllipse(-5, -5, 10, 10, pen, QBrush(C));
		temp->setFlag(QGraphicsEllipseItem::ItemIgnoresTransformations, true);
		temp->setPos(posX - m_iOffset, TF_HEIGHT - TF_MARGIN / 2 + 5);

		if (m_colorObjectList.size() != 0) {
			int idx = -1;
			for (int i = 0; i < m_colorObjectList.size(); i++) {
				if (m_colorObjectList.at(i)->pos().x() >= temp->pos().x()) {
					m_colorObjectList.insert(i, temp);
					idx = i;
					break;
				}
			}
			if (idx == -1)
				m_colorObjectList.append(temp);
		}
		else {
			m_colorObjectList.append(temp);
		}
	}

	inFile.close();

	setDefaultGeometry();

	// UPDATE COLOR and TF
	changeAll = true;
	updateColor();
	//emit sigChangeTF(m_listOTFObj, m_iOffset);
	
	//emit sigChangeTF(m_pTf);
}

/* mouse events for this(QGraphicsScene) */
void OTFWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
	QGraphicsScene::mouseMoveEvent(e);
	//QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	QPointF pt = e->lastScenePos();
	if (!m_isPressed) // mouse is just moving. (not pressed)
	{
		// [First] check if mouse is On Points.
		for (int otfIdx = 0; otfIdx < m_listOTFObj.size(); otfIdx++)
		{
			OTFPolygon* temp = m_listOTFObj.at(otfIdx);
			temp->setMinMax(m_min, m_max);
			for (int i = 0; i < temp->getPointList().size(); i++) {
				if (temp->getPointList().at(i)->contains(pt)) {
					//// deactivate previous selected polygons.
					//for( auto &i : m_selectedPolygonIdx )
					//	m_listOTFObj.at(i)->getPolygon()->deactivate();
					for (const auto& i : m_listOTFObj)	i->deactivateAll();
					temp->getPointList().at(i)->activate();
					// set color polygon.
					QBrush brush = temp->getPolygon()->brush();
					QColor color = brush.color();
					color.setAlphaF(0.3);
					brush.setColor(color);
					temp->getPolygon()->setBrush(brush);
					m_currElem._which = COMPONENT::POINT;
					m_currElem._iIdx = i;
					m_currElem._iPolyIdx = otfIdx;
					m_isOnPolygon = true;
					return;
				}
			}
		}
		// [Second] check if mouse is On Lines.
		for (int otfIdx = 0; otfIdx < m_listOTFObj.size(); otfIdx++)
		{
			OTFPolygon* temp = m_listOTFObj.at(otfIdx);
			temp->setMinMax(m_min, m_max);
			for (int i = 0; i < temp->getLineList().size(); i++) {
				// check if near the line.
				if (temp->getLineList().at(i)->contains(pt)) {
					//// deactivate previous selected polygons.
					//for( auto &i : m_selectedPolygonIdx )
					//	m_listOTFObj.at(i)->getPolygon()->deactivate();
					for (const auto& i : m_listOTFObj)	i->deactivateAll();
					temp->getLineList().at(i)->activate();
					// set color polygon.
					QBrush brush = temp->getPolygon()->brush();
					QColor color = brush.color();
					color.setAlphaF(0.3);
					brush.setColor(color);
					temp->getPolygon()->setBrush(brush);
					m_currElem._which = COMPONENT::LINE;
					m_currElem._iIdx = i;
					m_currElem._iPolyIdx = otfIdx;
					m_isOnPolygon = true;
					return;
				}
			}
		}
		// [Third] check if mouse is On Polygon.
		setDefaultGeometry();
		m_selectedPolygonIdx.clear();
		for (int otfIdx = 0; otfIdx < m_listOTFObj.size(); otfIdx++)
		{
			OTFPolygon* temp = m_listOTFObj.at(otfIdx);
			temp->setMinMax(m_min, m_max);
			if (temp->getPolygon()->contains(pt))
			{
				// set polygon activate.
				temp->getPolygon()->activate();

				m_currElem._which = COMPONENT::POLYGON;
				m_currElem._iIdx = -1;
				m_currElem._iPolyIdx = otfIdx;
				m_isOnPolygon = true;
				m_selectedPolygonIdx.push_back(otfIdx);
			}
		}
		if (!m_selectedPolygonIdx.empty())
			return;
		// [Fourth] check if mouse is On Color Object.
		for (int i = 0; i < m_colorObjectList.size(); i++) {
			if (std::abs(m_colorObjectList.at(i)->pos().x() - pt.x()) < 20
				&& std::abs(m_colorObjectList.at(i)->pos().y() - pt.y()) < 20)
			{
				setDefaultGeometry();
				m_colorObjectList.at(i)->setRect(-12, -10, 24, 20);
				m_currElem._which = COMPONENT::COLOR;
				m_currElem._iIdx = i;
				m_currElem._iPolyIdx = -1;
				m_isOnPolygon = true;
				return;
			}
		}
		// It is background.
		setDefaultGeometry();
		m_currElem._which = COMPONENT::NONE;
		m_currElem._iIdx = -1;
		m_currElem._iPolyIdx = -1;
		m_isOnPolygon = false;
	}
	else // mouse is dragging. (pressed)
	{
		OTFPolygon* temp;
		if (m_currElem._which == COMPONENT::POLYGON)
		{
			for (const int& i : m_selectedPolygonIdx) {
				temp = m_listOTFObj.at(i);
				temp->setMinMax(m_min, m_max);
				temp->movePolygon(pt.x() - m_currPoint.x(), m_iOffset);
			}
			m_currPoint = pt;

		}
		else if (m_currElem._which == COMPONENT::POINT)
		{
			temp = m_listOTFObj.at(m_currElem._iPolyIdx);
			temp->setMinMax(m_min, m_max);
			int idx = m_currElem._iIdx;
			if (idx == -1)	return;

			// move constraints.
			if (idx != temp->getPointList().size() - 1 && idx != 0) { // point is not end-points.
				// Y-axis constraints.
				if (pt.y() <= TF_MARGIN) { // if point is tend to go upper-limit.
					pt.setY(TF_MARGIN);
					m_currPoint.setY(temp->getPointList().at(idx)->y());
				}
				else if (pt.y() >= TF_HEIGHT - TF_MARGIN) { //if point is tend to go lower-limit.
					pt.setY(TF_HEIGHT - TF_MARGIN);
					m_currPoint.setY(temp->getPointList().at(idx)->y());
				}
				// X-axis constraints.
				if (pt.x() >= temp->getPointList().at(idx + 1)->x()) { // if point is tend to go right-limit.
					pt.setX(temp->getPointList().at(idx + 1)->x());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
				else if (pt.x() <= temp->getPointList().at(idx - 1)->x()) { // if point is tend to go left-limit.
					pt.setX(temp->getPointList().at(idx - 1)->x());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
			}
			else if (idx == 0) { // point is start-end point.
				// Y-axis constraints.
				//pt.setY(m_currPoint.y());
				pt.setY(TF_HEIGHT - TF_MARGIN);
				m_currPoint.setY(TF_HEIGHT - TF_MARGIN);
				// X-axis constraints.
				if (pt.x() >= temp->getPointList().at(idx + 1)->x()) {
					pt.setX(temp->getPointList().at(idx + 1)->x());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
				else if (pt.x() <= 0) {
					pt.setX(0);
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
			}
			else if (idx == temp->getPointList().size() - 1) { //point is end-end point.
				// Y-axis constraints.
				//pt.setY(m_currPoint.y());
				pt.setY(TF_HEIGHT - TF_MARGIN);
				m_currPoint.setY(TF_HEIGHT - TF_MARGIN);
				// X-axis constraints.
				if (pt.x() <= temp->getPointList().at(idx - 1)->x()) {
					pt.setX(temp->getPointList().at(idx - 1)->x());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
				else if (pt.x() > m_iTFSize) {
					pt.setX(m_iTFSize);
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
			}
			// actual point's movement.
			temp->movePoint(idx, pt.x() - m_currPoint.x(), pt.y() - m_currPoint.y(), m_iOffset);
			m_currPoint = pt;

			// display Alpha Value.
			QString tmpStr;
			tmpStr.append("  [" + QString().setNum((TF_HEIGHT - (int)m_currPoint.y() - TF_MARGIN) / (float)(TF_HEIGHT - 2 * TF_MARGIN) * 100.0f, 'f', 0) + "%]");
			m_currText->setPlainText(tmpStr);
			m_currText->setPos(m_currPoint.x() - 30, m_currPoint.y() - 25);
			m_currText->setVisible(true);

		}
		else if (m_currElem._which == COMPONENT::LINE)
		{
			temp = m_listOTFObj.at(m_currElem._iPolyIdx);
			temp->setMinMax(m_min, m_max);
			int idx = m_currElem._iIdx;
			if (idx == -1)	return;
			int ptIdx1 = idx + 1;
			int ptIdx2 = idx + 2;
			int dx = pt.x() - m_currPoint.x();
			int dy = pt.y() - m_currPoint.y();
			// move constraints. (Movement algorithm)
			EllipseItem_otf* point1 = temp->getPointList().at(ptIdx1);
			EllipseItem_otf* point2 = temp->getPointList().at(ptIdx2);
			// X-axis constraints.
			if (point1->x() + dx < temp->getPointList().at(ptIdx1 - 1)->x()) { // trying to go over left-limit.
				dx = temp->getPointList().at(ptIdx1 - 1)->x() - point1->x();
				pt.setX(m_currPoint.x() + dx);
			}
			else if (temp->getPointList().at(ptIdx2 + 1)->x() < point2->x() + dx) { // trying to go over right-limit.
				dx = temp->getPointList().at(ptIdx2 + 1)->x() - point2->x();
				pt.setX(m_currPoint.x() + dx);
			}
			// Y-axis constraints.
			// for Point #1.
			if (point1->y() + dy <= TF_MARGIN) { // trying to go over upper-limit.
				dy = TF_MARGIN - point1->y();
				pt.setY(m_currPoint.y() + dy);
			}
			else if (point1->y() + dy >= TF_HEIGHT - TF_MARGIN) { // trying to go over lower-limit.
				dy = TF_HEIGHT - TF_MARGIN - point1->y();
				pt.setY(m_currPoint.y() + dy);
			}
			// for Point #2.
			if (point2->y() + dy <= TF_MARGIN) { // trying to go over upper-limit.
				dy = TF_MARGIN - point2->y();
				pt.setY(m_currPoint.y() + dy);
			}
			else if (point2->y() + dy >= TF_HEIGHT - TF_MARGIN) { // trying to go over lower-limit.
				dy = TF_HEIGHT - TF_MARGIN - point2->y();
				pt.setY(m_currPoint.y() + dy);
			}

			temp->moveLine(idx, pt.x() - m_currPoint.x(), pt.y() - m_currPoint.y(), m_iOffset);
			m_currPoint = pt;

			// display Alpha Value.
			QString tmpStr;
			int val = static_cast<int>((point1->y() + point2->y()) / 2.0f);
			tmpStr.append("  [" + QString().setNum((TF_HEIGHT - val - TF_MARGIN) / (float)(TF_HEIGHT - 2 * TF_MARGIN) * 100.0f, 'f', 0) + "%]");
			m_currText->setPlainText(tmpStr);
			m_currText->setPos(m_currPoint.x() - 30, m_currPoint.y() - 25);
			m_currText->setVisible(true);


		}
		else if (m_currElem._which == COMPONENT::COLOR)
		{
			// Color Object Update.
			if (m_currElem._iIdx == -1)	return;
			QGraphicsEllipseItem* tmpRect = m_colorObjectList.at(m_currElem._iIdx);
			if (pt.x() < 0)			pt.setX(0);
			if (pt.x() > m_iTFSize)	pt.setX(m_iTFSize);
			tmpRect->moveBy(pt.x() - m_currPoint.x(), 0);
			m_currPoint = pt;

			// sorting. ( sort m_colorObjectList components - by positions )
			//auto Comparing = [](QGraphicsEllipseItem *item1, QGraphicsEllipseItem *item2) { return item1->x() > item2->x(); };
			//qSort(m_colorObjectList.begin(), m_colorObjectList.end(), Comparing);
			/*
			QList<QGraphicsEllipseItem*> tmpList;
			while( !m_colorObjectList.isEmpty() )
			{
				int idx = 0;
				float minVal = m_colorObjectList.at(idx)->x();
				QGraphicsEllipseItem *item = m_colorObjectList.at(idx);
				for( int i=1; i<m_colorObjectList.size(); i++ ){
					if( m_colorObjectList.at(i)->x() < minVal ){
						idx = i;
						minVal = m_colorObjectList.at(idx)->x();
						item = m_colorObjectList.at(idx);
					}
				}
				m_colorObjectList.removeAt(idx);
				tmpList.append(item);
			}
			m_colorObjectList = tmpList;*/

			// update.
			updateColor();
			
			return;
		}
		else
			return;

		emit sigChangeTF(m_listOTFObj, m_iOffset);

		
	}
}

// set default geometry (when mouse leave or non-overlapped)
void OTFWidget::setDefaultGeometry(void)
{
	/*for( int objIdx=0; objIdx<m_listOTFObj.size(); objIdx++ )
	{
		CAzOTFPolygon *temp = m_listOTFObj.at(objIdx);
		QBrush brush = temp->getPolygon()->brush();
		QColor color(Qt::red);
		color.setAlphaF(0.1);
		brush.setColor(color);
		temp->getPolygon()->setBrush(brush);

		QPen pen = temp->getPolygon()->pen();
		pen.setColor(Qt::red);
		pen.setWidth(1);
		temp->getPolygon()->setPen(pen);

		for(int i=0; i<temp->getPointList().size(); i++)
			temp->getPointList().at(i)->setRect(-5, -5, 10, 10);
		for(int i=0; i<m_colorObjectList.size(); i++)
			m_colorObjectList.at(i)->setRect(-10, -5, 20, 10);
	}*/
	for (const auto& i : m_listOTFObj)
		i->deactivateAll();
	for (const auto& i : m_colorObjectList)
		i->setRect(-10, -5, 20, 10);
}

void OTFWidget::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
	QGraphicsScene::mousePressEvent(e);
	m_isPressed = true;
	QPointF pt = e->lastScenePos();
	if (m_isOnPolygon == false && !m_bTextPressed)
		emit sigSetScrollHandDrag();
	else if (e->button() == Qt::LeftButton)
	{
		if(m_isOnPolygon)
			emit sigOTFClicked();
		//isOnPolygon = isPointOnPolygon(pt);
	}
	m_currPoint = pt;
}

void OTFWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
	QGraphicsScene::mouseReleaseEvent(e);
	m_currText->setVisible(false);
	m_isPressed = false;

	m_currPoint = e->lastScenePos();
	QPoint pos(e->screenPos().x(), e->screenPos().y());
	CURR_COMPONENT currComponent = getCurrComponent(m_currPoint);
	if (e->button() == Qt::LeftButton) {
		if (m_isOnPolygon) {
			emit sigOTFReleased();
		}
		
			//emit sigChangeTF(m_pTf);

		//switch( currComponent )
		//{
		//case (CURR_COMPONENT::BACKGROUND):
		//case (CURR_COMPONENT::COLORTAB):
		//	break;
		//case (CURR_COMPONENT::POINT):
		//case (CURR_COMPONENT::LINE):
		//case (CURR_COMPONENT::POLYGON):
		//case (CURR_COMPONENT::COLOROBJ):
		//	emit sigRenderHighResolution();
		//	break;
		//}
		
	}
	else if (e->button() == Qt::RightButton) {
		m_menu->clear();
		switch (currComponent)
		{
		case (CURR_COMPONENT::BACKGROUND):
			m_menu->addSeparator();
			m_menu->addAction(m_addPolygonAct);
			m_menu->popup(pos);
			break;
		case (CURR_COMPONENT::POINT):
			m_menu->addAction(m_removePointAct);
			m_menu->popup(pos);
			break;
		case (CURR_COMPONENT::LINE):
			break;
		case (CURR_COMPONENT::POLYGON):
			m_menu->addAction(m_addPointAct);
			m_menu->addSeparator();
			m_menu->addAction(m_removePolygonAct);
			m_menu->popup(pos);
			break;
		case (CURR_COMPONENT::COLOROBJ):
			m_menu->addAction(m_updateColorObjectAct);
			m_menu->addAction(m_removeColorObjectAct);
			m_menu->popup(pos);
			break;
		case (CURR_COMPONENT::COLORTAB):
			m_menu->addAction(m_addColorObjectAct);
			m_menu->popup(pos);
			break;
		}
	}
}

void OTFWidget::slotLeave()
{
	m_currLine->setVisible(false);
	m_currText->setVisible(false);

	setDefaultGeometry();
}


CURR_COMPONENT OTFWidget::getCurrComponent(QPointF point)
{
	// classify which component was selected.
	if (point.y() >= TF_HEIGHT - TF_MARGIN) // check if color tab,
	{
		if (m_currElem._which == COMPONENT::COLOR)
			return CURR_COMPONENT::COLOROBJ;
		//for(int i=0; i<m_colorObjectList.size(); i++) {
		//	// if color object.
		//	if( std::abs(m_colorObjectList.at(i)->pos().x() - point.x()) < 7
		//		&& std::abs(m_colorObjectList.at(i)->pos().y() - point.y()) < 7 )
		//		return CURR_COMPONENT::COLOROBJ;
		//}
		return CURR_COMPONENT::COLORTAB;
	}
	// check if polygon,
	else
	{
		for (int objIdx = 0; objIdx < m_listOTFObj.size(); objIdx++)
		{

			for (int i = 0; i < m_listOTFObj.at(objIdx)->getPointList().size(); i++) {
				// if point.
				if (std::abs(m_listOTFObj.at(objIdx)->getPointList().at(i)->pos().x() - point.x()) < 7
					&& std::abs(m_listOTFObj.at(objIdx)->getPointList().at(i)->pos().y() - point.y()) < 7)
					return CURR_COMPONENT::POINT;
			}
			// if line.
			// if polygon.
			if (m_listOTFObj.at(objIdx)->getPolygon()->contains(point))
				return CURR_COMPONENT::POLYGON;
		}
		// background
		return CURR_COMPONENT::BACKGROUND;
	}
}

void OTFWidget::loadDefault(void)
{
	OTFPolygon* tmpObj = new OTFPolygon();
	//tmpObj->setDefaultPolygon(QPointF(246, 255), 0);
	
	
	tmpObj->addToScene(this);
	// update to m_listOTFObj <CAzOTFPolygon>
	m_listOTFObj.append(tmpObj);
	
	//float pt1 = tmpObj->getPointList()[0]->pos().x();

	emit sigChangeTF(m_listOTFObj, m_iOffset);
}

void OTFWidget::loadBone(void)
{
	for (int i = 0; i < m_listOTFObj.size(); i++) {
		OTFPolygon* temp = m_listOTFObj.at(i);
		temp->removeFromScene(this);
		delete (m_listOTFObj.at(i));
		m_listOTFObj.replace(i, nullptr);
	}
	m_listOTFObj.clear();
	m_colorList.clear();
	for (int i = 0; i < m_colorObjectList.size(); i++)
		this->removeItem(m_colorObjectList.at(i));
	m_colorObjectList.clear();

	OTFPolygon* tmpObj = new OTFPolygon();
	tmpObj->setBonePresetPolygon(m_iOffset);
	addColorObject(200, QColor(Qt::white));

	tmpObj->addToScene(this);
	// update to m_listOTFObj <CAzOTFPolygon>
	m_listOTFObj.append(tmpObj);

	emit sigChangeTF(m_listOTFObj, m_iOffset); 
	updateColor();

}

void OTFWidget::loadClothed(void)
{
	for (int i = 0; i < m_listOTFObj.size(); i++) {
		OTFPolygon* temp = m_listOTFObj.at(i);
		temp->removeFromScene(this);
		delete (m_listOTFObj.at(i));
		m_listOTFObj.replace(i, nullptr);
	}
	m_listOTFObj.clear();
	m_colorList.clear();
	for (int i = 0; i < m_colorObjectList.size(); i++)
		this->removeItem(m_colorObjectList.at(i));
	m_colorObjectList.clear();

	OTFPolygon* tmpObj = new OTFPolygon();
	tmpObj->setClothedPresetPolygon();
	addColorObject(200, QColor(Qt::white));

	tmpObj->addToScene(this);
	// update to m_listOTFObj <CAzOTFPolygon>
	m_listOTFObj.append(tmpObj);

	emit sigChangeTF(m_listOTFObj, m_iOffset);
	updateColor();
}

//OTFPolygon* OTFWidget::getOTFObject(int index) {
//	return this->m_listOTFObj[index];
//}


void OTFWidget::slotAddPolygon(void) {

	OTFPolygon *tmpObj = new OTFPolygon();
	tmpObj->setDefaultPolygon(m_currPoint, m_iOffset);
	tmpObj->addToScene(this);
	// update to m_listOTFObj <CAzOTFPolygon>
	m_listOTFObj.append(tmpObj);
	emit sigChangeTF(m_listOTFObj, m_iOffset);
}

void OTFWidget::slotRemovePolygon(void)
{
	for (const int& i : m_selectedPolygonIdx) {
		OTFPolygon* temp = m_listOTFObj.at(i);
		temp->removeFromScene(this);
		delete(m_listOTFObj.at(i));
		m_listOTFObj.replace(i, nullptr);
	}

	for (int i = 0; i < m_listOTFObj.size(); ) {
		if (m_listOTFObj.at(i) == nullptr)
			m_listOTFObj.removeAt(i);
		else
			i++;
	}

	emit sigChangeTF(m_listOTFObj, m_iOffset);
}


void OTFWidget::slotAddPoint(void) {

	OTFPolygon* tmpObj = m_listOTFObj.at(m_currElem._iPolyIdx);
	tmpObj->addPoint(m_currPoint, m_iOffset, this);

	m_listOTFObj.replace(m_currElem._iPolyIdx, tmpObj);
	m_currElem._which = COMPONENT::NONE;
	m_currElem._iIdx = -1;
	m_currElem._iPolyIdx = -1;

	emit sigChangeTF(m_listOTFObj, m_iOffset);
}

void OTFWidget::slotRemovePoint(void) {
	OTFPolygon* tmpObj;
	int polyIdx = -1;
	int pointIdx = -1;
	float dist = std::numeric_limits<float>::max();
	QPointF point(m_currPoint);

	for (int objIdx = 0; objIdx < m_listOTFObj.size(); objIdx++) {

		tmpObj = m_listOTFObj.at(objIdx);
		for (int i = 0; i < tmpObj->getPointList().size(); i++) {
			float currDist = std::pow(tmpObj->getPointList().at(i)->pos().x() - point.x(), 2.0) + std::pow(tmpObj->getPointList().at(i)->pos().y() - point.y(), 2.0);
			if (currDist < dist) {
				dist = currDist;
				polyIdx = objIdx;
				pointIdx = i;
			}
		}
	}

	if (polyIdx == -1 || pointIdx == -1) {
		return;
	}

	tmpObj = m_listOTFObj.at(polyIdx);
	tmpObj->removePoint(pointIdx, this);

	m_listOTFObj.replace(polyIdx, tmpObj);
	m_currElem._which = COMPONENT::NONE;
	m_currElem._iIdx = -1;
	m_currElem._iPolyIdx = -1;

	// remove OTF Object if insufficient points left.
	if (tmpObj->getPointList().size() < 3) {
		tmpObj->removeFromScene(this);
		m_listOTFObj.removeAt(polyIdx);
		//m_listOTFObj.replace(i, nullptr);
	}

	emit sigChangeTF(m_listOTFObj, m_iOffset);
}

void OTFWidget::slotAddColor(void) {
	QPointF point(m_currPoint);
	QColor color = QColorDialog::getColor();

	if (!color.isValid())	return;
	// add new.
	addColorObject(point.x(), color);
	// update.
	updateColor();
	//emit sigChangeTF(m_pTf);
}

void OTFWidget::slotRemoveColor(void) {
	if (m_currElem._which != COMPONENT::COLOR) return;
	this->removeItem(m_colorObjectList.at(m_currElem._iIdx));
	m_colorObjectList.removeAt(m_currElem._iIdx);
	updateColor();
	//emit sigChangeTF(m_pTf);
}

void OTFWidget::slotUpdateColor(void)
{
	QColor color = QColorDialog::getColor();
	if (!color.isValid()) {
		m_isPressed = false;
		return;
	}
	m_colorObjectList.at(m_currElem._iIdx)->setBrush(color);
	QPen pen;
	pen.setColor(Qt::gray);
	if (color.red() > 125 && color.green() > 125 && color.blue() > 125)
		pen.setColor(Qt::black);
	m_colorObjectList.at(m_currElem._iIdx)->setPen(pen);
	updateColor();
}

void OTFWidget::addColorObject(int posX, QColor color)
{
	QGraphicsEllipseItem* temp;
	QPen pen;
	pen.setColor(Qt::gray);
	if (color.red() > 125 && color.green() > 125 && color.blue() > 125) {
		pen.setColor(Qt::black);
	}
	temp = addEllipse(-10, -5, 20, 10, pen, QBrush(color));
	temp->setFlag(QGraphicsEllipseItem::ItemIgnoresTransformations, true);
	temp->setPos(posX, TF_HEIGHT - TF_MARGIN / 2 + 5);

	if (m_colorObjectList.size() != 0) {
		int idx = -1;
		for (int i = 0; i < m_colorObjectList.size(); i++) {
			if (m_colorObjectList.at(i)->pos().x() >= temp->pos().x()) {
				m_colorObjectList.insert(i, temp);
				idx = i;
				break;
			}
		}
		if (idx == -1)
			m_colorObjectList.append(temp);
	}
	else {
		m_colorObjectList.append(temp);
	}

}


void OTFWidget::updateColor() {
	// use tmpList by sorting & displaying.
	QList<QGraphicsEllipseItem*> tmpList;
	for (int i = 0; i < m_colorObjectList.size(); i++)
		tmpList.append(m_colorObjectList.at(i));
	auto Comparing = [](QGraphicsEllipseItem* item1, QGraphicsEllipseItem* item2) { return item1->x() < item2->x(); };
	qSort(tmpList.begin(), tmpList.end(), Comparing);

	if (tmpList.size() != 0) {
		QPointF start(0.0, 0.0), last(m_iTFSize, 0.0);
		if (tmpList.at(0)->pos().x() < 0.0)
			start.setX(tmpList.at(0)->x());
		if (tmpList.last()->pos().x() > m_iTFSize)
			last.setX(tmpList.last()->pos().x());

		QLinearGradient linearGrad(start, last);
		linearGrad.setColorAt(0, tmpList.at(0)->brush().color());
		for (int i = 0; i < tmpList.size(); i++)
			linearGrad.setColorAt((tmpList.at(i)->pos().x()) / static_cast<float>(last.x() - start.x()), tmpList.at(i)->brush().color());
		linearGrad.setColorAt(1, tmpList.last()->brush().color());

		QBrush brush(linearGrad);
		m_mainColorRect->setBrush(brush);

		m_colorList.clear();
		for (int i = 0; i < tmpList.size(); i++) {
			OTFColor colorpoint;

			colorpoint._intensity = tmpList.at(i)->pos().x();
			colorpoint._color = tmpList.at(i)->brush().color();

			m_colorList.append(colorpoint);

		}

		if (changeAll) {
			emit sigChangeAll(m_colorList, m_listOTFObj, m_iOffset);
			changeAll = false;
		}
		emit sigChangeColor(m_colorList);
	}
	else {
		m_mainColorRect->setBrush(QBrush());
		m_colorList.clear();
		return;
	}

}	

