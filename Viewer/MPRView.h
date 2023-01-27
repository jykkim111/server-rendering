#pragma once

#include <QWidget>
#include <QLayout>
#include "SliceWidget.h"
#include "DVRWidget.h"

class MPRView : public QWidget {

	Q_OBJECT
public:
	MPRView(QWidget* parent, SliceWidget* axial_slice, SliceWidget* coronal_slice, SliceWidget* sagittal_slice, DVRWidget* dvr_slice);


private:
	QStackedLayout* stackedWidget = new QStackedLayout;
	SliceWidget* m_axial_slice, * m_coronal_slice, * m_sagittal_slice;
	DVRWidget* m_dvr_slice;
	QWidget* tab;

	QPushButton* close, * axial_btn, * sagittal_btn, * coronal_btn;
};


