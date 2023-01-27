#pragma once
#include <QWidget>
#include <QHBoxLayout>
#include <QtCharts>
#include <QPainter>
#include <QLayout>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qevent.h>
#include <qlabel.h>
#include <qgroupbox.h>

#include "EllipseItem_color.h"
#include "OTFPolygon.h"
#include "TextItem.h"
#include "OTF.h"
#include "WidgetText.h"

class OTFWidget : public QGraphicsScene
{

	Q_OBJECT

public:
	OTFWidget(QWidget* parent = 0);
	~OTFWidget();

public:
	void initOTF(int min, int max, int offset);
	void initLoadPresets(void);
	//OTFPolygon* OTFWidget::getOTFObject(int index);
	void loadBone(void);
	void loadClothed(void);
	inline QStringList getTFPath(void) { return m_listTFPaths; }

protected:
	// protected overriding functions for mouse events.
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;


signals:
	void sigChangeTF(QList<OTFPolygon*>, int offset);
	void sigChangeColor(QList<OTFColor>);
	void sigChangeAll(QList<OTFColor>, QList<OTFPolygon*>, int offset);
	void sigOTFClicked();
	void sigOTFReleased();
	void sigSetScrollHandDrag();
	void sigOTFPresetLoad();

public slots:
	void	slotViewPortChanged(float);
	void	slotAddPolygon(void);
	void	slotRemovePolygon(void);
	void	slotAddPoint(void);
	void	slotRemovePoint(void);
	void	slotAddColor(void);
	void	slotRemoveColor(void);
	void	slotUpdateColor(void);
	void	slotLeave(void);
	void	slotExport(void);
	void	slotLoadPreset(void);
	void	slotSelected(int);

private:
	CURR_COMPONENT getCurrComponent(QPointF point);
	void setDefaultGeometry(void);
	void loadPresets(const std::string& path);
	void loadDefault(void);
	void addColorObject(int posX, QColor color);
	//void updateTF(void);			// Update Alpha value.
	void updateColor(void);		// Update Color value.



private:
	WidgetText*				m_pTextExport;
	WidgetText*				m_pTextPreset;
	QGraphicsPolygonItem*	m_histogram;
	QList<OTFPolygon*>		m_listOTFObj;
	QGraphicsLineItem		*m_currLine;
	QGraphicsTextItem		*m_currText;
	QPointF					m_currPoint;

	//Mouse interaction compoenents

	bool				m_bTextPressed;
	bool				m_isPressed;
	bool				m_isOnPolygon;
	ELEMENT				m_currElem;
	std::vector<int>	m_selectedPolygonIdx;

	QList<OTFColor>					m_colorList;		// TF color control point list
	QList<QGraphicsEllipseItem*>	m_colorObjectList;	// represents color for current density.

	//main color rect.
	QGraphicsRectItem*	m_mainColorRect;
	int					m_iTFSize;
	int					m_iOffset;
	int					m_min;
	int					m_max;

	QPointF				current_point;
	bool				is_pressed;
	
	// TF resource handling members.
	QStringList				m_listTFPaths;
	bool				changeAll = false;

	// Menu and Actions.
	QMenu* m_menu;
	QAction* m_addPolygonAct;
	QAction* m_removePolygonAct;
	QAction* m_addPointAct;
	QAction* m_removePointAct;
	QAction* m_addColorObjectAct;
	QAction* m_updateColorObjectAct;
	QAction* m_removeColorObjectAct;

};


