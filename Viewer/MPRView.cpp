#include "MPRView.h"

MPRView::MPRView(QWidget* parent, SliceWidget* axial_slice, SliceWidget* coronal_slice, SliceWidget* sagittal_slice, DVRWidget* dvr_slice) : QWidget(parent) {

	m_axial_slice = axial_slice;
	m_coronal_slice = coronal_slice;
	m_sagittal_slice = sagittal_slice;
	m_dvr_slice = dvr_slice;

	QHBoxLayout* overall_layout = new QHBoxLayout;
	QHBoxLayout* view_layout = new QHBoxLayout;
	QVBoxLayout* left_layout = new QVBoxLayout;
	left_layout->addWidget(axial_slice);
	left_layout->addWidget(coronal_slice);
	QVBoxLayout* right_layout = new QVBoxLayout;
	right_layout->addWidget(sagittal_slice);
	right_layout->addWidget(dvr_slice);
	
	view_layout->addLayout(left_layout);
	view_layout->addLayout(right_layout);


	tab = new QWidget;
	QHBoxLayout* tab_layout = new QHBoxLayout(tab);
	tab_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	tab_layout->setMargin(0);
	close = new QPushButton;
	close->setIcon(QIcon("Images/Slice/out.png"));
	close->setIconSize(QSize(15, 15));
	axial_btn = new QPushButton;
	axial_btn->setText("Axial");
	sagittal_btn = new QPushButton;
	sagittal_btn->setText("Sagittal");
	coronal_btn = new QPushButton;
	coronal_btn->setText("Coronal");

	tab_layout->addWidget(axial_btn);
	tab_layout->addWidget(sagittal_btn);
	tab_layout->addWidget(coronal_btn);
	tab_layout->addWidget(close);
	
	overall_layout->addWidget(tab);
	overall_layout->addLayout(view_layout);

	m_axial_slice->show();
	m_sagittal_slice->show();
	m_coronal_slice->show();
	m_dvr_slice->show();

}



