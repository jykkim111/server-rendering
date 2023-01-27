#include "OTFView.h"

#include<qevent.h>

#include <algorithm>
#include <vector>
#include <iostream>

OTFView::OTFView(QWidget* parent)
	: QGraphicsView(parent)
{
	setMouseTracking(true);
	this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	//this->setStyleSheet( QString("QScrollBar:horizontal { border: 2px solid grey; background: #32CC99; height: 15px; margin: 0px 20px 0 20px; }") );
	this->setStyleSheet(QString(
		"QScrollBar::handle:horizontal { background: rgb(250, 50, 50); }"
		//"QScrollBar::handle:horizontal { background: black; }"
		"QScrollBar:horizontal { background: white; }"
		//"QScrollBar::handle:sub-control { ; }"
	));
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QSizePolicy otfSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	otfSizePolicy.setHorizontalStretch(0);
	otfSizePolicy.setVerticalStretch(0);
	otfSizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
	setSizePolicy(otfSizePolicy);
	setMinimumSize(QSize(0, 200));
	setMaximumSize(QSize(16777215, 200));
}

OTFView::~OTFView(void)
{
}

void OTFView::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);
	this->setDragMode(NoDrag);
}

void OTFView::wheelEvent(QWheelEvent *event)
{
	static int offset = 0;
	double numDegrees = event->delta() / 8.0; 
	double numSteps = numDegrees / 15.0;
	double factor = std::pow(1.125, numSteps);
	offset += (factor > 1.0f ? 1 : -1);
	if (offset < -10) {
		offset++;	return;
	}
	if (offset > 15) {
		offset--;	return;
	}

	this->scale(factor, 1);
	QGraphicsView::wheelEvent(event);
}

void OTFView::leaveEvent(QEvent* event)
{
	emit sigLeave();
	QGraphicsView::leaveEvent(event);
}

void OTFView::resizeEvent(QResizeEvent* pEvent)
{
	QGraphicsView::resizeEvent(pEvent);
}

void OTFView::slotSetScrollHandDrag(void)
{
	this->setDragMode(ScrollHandDrag);
}

bool OTFView::viewportEvent(QEvent* event)
{
	QGraphicsView::viewportEvent(event);

	QPointF pt = this->mapToScene(0, 0);
	emit sigViewPortChanged(pt.x());

	return true;
}
