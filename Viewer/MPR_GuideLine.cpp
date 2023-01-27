#include "MPR_GuideLine.h"

MPR_GuideLine::MPR_GuideLine(SliceType slice_type) {

	m_center_ellipse = new QGraphicsEllipseItem();
	QPen pen_forCenter(Qt::yellow);
	pen_forCenter.setCosmetic(true);
	pen_forCenter.setWidth(3.0f);
	m_center_ellipse->setZValue(5.0f);
	m_center_ellipse->setPen(pen_forCenter);
	m_center_ellipse->setRect(-20, -20, 40, 40);
	m_center_ellipse->setAcceptHoverEvents(true);
	m_center_ellipse->setVisible(false);


	m_center_ellipse_pt = new QGraphicsEllipseItem();
	m_center_ellipse_pt->setPen(pen_forCenter);
	m_center_ellipse_pt->setRect(-0.75, -0.75, 1.5f, 1.5f);
	m_center_ellipse_pt->setAcceptHoverEvents(true);
	m_center_ellipse_pt->setVisible(false);


	QPen pen_forVerticalLine, pen_forHorizontalLine, pen_forThicknessV, pen_forThicknessH;
	pen_forVerticalLine.setCosmetic(true);
	pen_forHorizontalLine.setCosmetic(true);
	pen_forThicknessV.setCosmetic(true);
	pen_forThicknessH.setCosmetic(true);

	m_guidelines_hL = new QGraphicsLineItem();
	m_guidelines_hR = new QGraphicsLineItem();
	m_guidelines_vT = new QGraphicsLineItem();
	m_guidelines_vB = new QGraphicsLineItem();
	m_thicknessLines_hL = new QGraphicsLineItem();
	m_thicknessLines_hR = new QGraphicsLineItem();
	m_thicknessLines_vT = new QGraphicsLineItem();
	m_thicknessLines_vB = new QGraphicsLineItem();

	m_thicknessLines_hL->setOpacity(0.7f);
	m_thicknessLines_hR->setOpacity(0.7f);
	m_thicknessLines_vT->setOpacity(0.7f);
	m_thicknessLines_vB->setOpacity(0.7f);
	
	//penThickness.setWidthF(5.0f);
	//penThickness.setColor(Qt::yellow);

	switch (slice_type) {
	case AXIAL_SLICE:
		pen_forVerticalLine.setColor(QColor(0, 0xFF, 0, 0x60));
		pen_forHorizontalLine.setColor(QColor(0, 0, 0xFF, 0x60));
		break;
	case SAGITTAL_SLICE:
		pen_forVerticalLine.setColor(QColor(0, 0, 0xFF, 0x60));
		pen_forHorizontalLine.setColor(QColor(0xFF, 0, 0, 0x60));
		break;
	case CORONAL_SLICE:
		pen_forVerticalLine.setColor(QColor(0, 0xFF, 0, 0x60));
		pen_forHorizontalLine.setColor(QColor(0xFF, 0, 0, 0x60));
		break;
	}


	m_guidelines_hL->setPen(pen_forHorizontalLine);
	m_guidelines_hR->setPen(pen_forHorizontalLine);
	m_guidelines_vT->setPen(pen_forVerticalLine);
	m_guidelines_vB->setPen(pen_forVerticalLine);
	m_thicknessLines_hL->setPen(pen_forHorizontalLine);
	m_thicknessLines_hR->setPen(pen_forHorizontalLine);
	m_thicknessLines_vT->setPen(pen_forVerticalLine);
	m_thicknessLines_vB->setPen(pen_forVerticalLine);

	m_guidelines_hL->setVisible(false);
	m_guidelines_hR->setVisible(false);
	m_guidelines_vT->setVisible(false);
	m_guidelines_vB->setVisible(false);
	m_thicknessLines_hL->setVisible(false);
	m_thicknessLines_hR->setVisible(false);
	m_thicknessLines_vT->setVisible(false);
	m_thicknessLines_vB->setVisible(false);

	
	std::cout << "initialized guideline" << std::endl;
}
/*
void MPR_GuideLine::initGuideLine(const QPointF ptCenter) {
	m_angle = 0.0f;
	m_center_point = ptCenter;
	this->setPos(ptCenter.x(), ptCenter.y());


	for (int i = 0; i < 2; i++) {
		m_guidelines_h[i]->setVisible(true);
		m_guidelines_v[i]->setVisible(true);
	}
}
*/

void MPR_GuideLine::addLineToScene(QGraphicsScene* pScene) {
	std::cout << "adding guideline components to scene... " << std::endl;

	pScene->addItem(m_guidelines_hL);
	pScene->addItem(m_guidelines_hR);
	pScene->addItem(m_guidelines_vT);
	pScene->addItem(m_guidelines_vB);
	pScene->addItem(m_thicknessLines_hL);
	pScene->addItem(m_thicknessLines_hR);
	pScene->addItem(m_thicknessLines_vT);
	pScene->addItem(m_thicknessLines_vB);

	pScene->addItem(m_center_ellipse);
	pScene->addItem(m_center_ellipse_pt);
}


void MPR_GuideLine::setHorizontalLineWidth(const float width)
{
	QPen pen = m_guidelines_hL->pen();
	pen.setWidth(width);
	m_guidelines_hL->setPen(pen);
	m_guidelines_hR->setPen(pen);
}

void MPR_GuideLine::setVerticalLineWidth(const float width)
{

	QPen pen = m_guidelines_vT->pen();
	pen.setWidth(width);
	m_guidelines_vT->setPen(pen);
	m_guidelines_vB->setPen(pen);
}

void MPR_GuideLine::setHorizontalThickness(const float thickness) {
	QPen pen = m_thicknessLines_hR->pen();
	pen.setWidth(thickness);
	m_thicknessLines_hR->setPen(pen);
	m_thicknessLines_hL->setPen(pen);
}

void MPR_GuideLine::setVerticalThickness(const float thickness) {
	QPen pen = m_thicknessLines_vT->pen();
	pen.setWidth(thickness);
	m_thicknessLines_vT->setPen(pen);
	m_thicknessLines_vB->setPen(pen);
}






void MPR_GuideLine::rotate(const float angle) {
	m_angle += angle;
	if (m_angle > 360) m_angle -= 360;
	if (m_angle < 0) m_angle += 360;

	m_guidelines_hL->setRotation(angle);
	m_guidelines_hR->setRotation(angle);
	m_guidelines_vT->setRotation(angle);
	m_guidelines_vB->setRotation(angle);
	m_thicknessLines_hL->setRotation(angle);
	m_thicknessLines_hR->setRotation(angle);
	m_thicknessLines_vT->setRotation(angle);
	m_thicknessLines_vB->setRotation(angle);
}

void MPR_GuideLine::setCenter(QPointF ptCenter) {
	m_center_point = ptCenter;
	m_center_ellipse->setPos(ptCenter);
	m_center_ellipse_pt->setPos(ptCenter);

	m_guidelines_hL->setPos(ptCenter);
	m_guidelines_hR->setPos(ptCenter);
	m_guidelines_vT->setPos(ptCenter);
	m_guidelines_vB->setPos(ptCenter);
	m_thicknessLines_hL->setPos(ptCenter);
	m_thicknessLines_hR->setPos(ptCenter);
	m_thicknessLines_vT->setPos(ptCenter);
	m_thicknessLines_vB->setPos(ptCenter);
}

void MPR_GuideLine::setLine(const QPointF ptCenter, const float center_radius, const float mTranslate, const float mRotate) {

	m_margin_translate = mTranslate;
	float length = mTranslate * 20;

	// index 0 is left side line for horizontal
	m_guidelines_hL->setLine(-length, 0, -center_radius, 0);
	m_guidelines_hR->setLine(center_radius, 0, length, 0);
	m_guidelines_vT->setLine(0, -length, 0, -center_radius);
	m_guidelines_vB->setLine(0, center_radius, 0, length);

	m_thicknessLines_hL->setLine(-length, 0, -center_radius, 0);
	m_thicknessLines_hR->setLine(center_radius, 0, length, 0);
	m_thicknessLines_vT->setLine(0, -length, 0, -center_radius);
	m_thicknessLines_vB->setLine(0, center_radius, 0, length);
}

void MPR_GuideLine::setLineVisible(bool click) {
	m_guidelines_hL->setVisible(click);
	m_guidelines_hR->setVisible(click);
	m_guidelines_vT->setVisible(click);
	m_guidelines_vB->setVisible(click);
	m_thicknessLines_hL->setVisible(click);
	m_thicknessLines_hR->setVisible(click);
	m_thicknessLines_vT->setVisible(click);
	m_thicknessLines_vB->setVisible(click);
}

void MPR_GuideLine::setCenterEllipseVisible(bool click) {
	m_center_ellipse->setVisible(click);
	m_center_ellipse_pt->setVisible(click);
}
