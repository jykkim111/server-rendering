#pragma once
#include <qaction.h>
#include <qgraphicsview.h>

#include "../CgipCommon/CgipPoint.h"
#include "../ViewerManager/global.h"


class OTFView : public QGraphicsView
{
	Q_OBJECT
public:
	OTFView(QWidget* parent = 0);
	~OTFView(void);

signals:
	void sigLeave(void);
	void sigViewPortChanged(float x);

protected:
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void resizeEvent(QResizeEvent* pEvent) override;
	virtual bool viewportEvent(QEvent* event) override;

public slots:
	void slotSetScrollHandDrag(void);

private:

};


