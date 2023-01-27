#pragma once

#include <QWidget>
#include <QLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QLayoutItem>
#include <qgraphicsscene.h>
#include "../ViewerManager/global.h"
#include "../Viewer/OTFWidget.h"
#include "OTFView.h"
#include "OTFPresetViewer.h"

class ViewLayout : public QWidget {
	Q_OBJECT
	public:
		ViewLayout(QWidget *parent);

		void setViewLayout(QLayout* layout);

		void setMPRDVRLayout(QWidget*, QWidget*, QWidget*, QWidget*,int l = 50);
		void setMPRLayoutVH(QWidget *view1, QWidget *view2, QWidget *view3);
		void setMPRLayoutHV(QWidget *view1, QWidget *view2, QWidget *view3, int l = 65);

		void setViewFullScreen(QWidget* view1, QWidget* view2, QWidget* view3, SliceType slice_type);
		void setViewFullScreen(QWidget* view1, QWidget* view2, QWidget* view3, QWidget* view4, SliceType slice_type);
		void setViewFullScreen2(QWidget* view1, QWidget* view2, QWidget* view3, SliceType slice_type);

		bool setCPRLayout(QWidget*, QWidget*, QWidget*, SliceType, int l = 50);

		void setOTFView(QWidget* view1, QWidget* view2, QWidget* view3, QWidget* view4);
		void resetOTF(VolumePacket volume);
		

	signals:
		void sigChangeViewLayout();
		void sigChangeAxialLayout();
		void sigChangeSagittalLayout();
		void sigChangeCoronalLayout();

		void sigChangeAll(QList<OTFColor>, QList<OTFPolygon*>, int offset);
		void sigChangeTF(QList<OTFPolygon*>, int offset);
		void sigChangeColor(QList<OTFColor>);
		void sigOTFClicked();
		void sigOTFReleased();

	private:
		QLayout* m_view_layout;
		QWidget* tab;
		OTFView* m_otf_window;
		OTFWidget* m_otf_view;
		QGraphicsView* otf_view;
		QHBoxLayout* otf_layout;
		QPushButton* close, * axial_btn, * sagittal_btn, * coronal_btn;
		OTFPresetViewer* m_pOTFPresetViewer = nullptr;
	
	public slots:
		void slotChangeTF(QList<OTFPolygon*> polyList, int offset);
		void slotChangeAll(QList<OTFColor> colorList, QList<OTFPolygon*> polyList, int offset);
		void slotOTFClicked();
		void slotOTFReleased();
		void slotChangeColor(QList<OTFColor> colorList);
		void slotShowOTFPreset(void);
};