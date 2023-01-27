#include "ViewLayout.h"

ViewLayout::ViewLayout(QWidget* parent) : QWidget(parent) {

	m_view_layout = nullptr;
	QVBoxLayout* layout = new QVBoxLayout;
	this->setLayout(layout);

	QSizePolicy size_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	size_policy.setHorizontalStretch(0);
	size_policy.setVerticalStretch(0);
	setSizePolicy(size_policy);

	this->setMouseTracking(true);

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

	m_otf_window = new OTFView();
	m_otf_view = new OTFWidget();
	m_otf_window->setScene(m_otf_view);
	//m_otf_view->initOTF(-1024, 1771, -1024);




	connect(close, SIGNAL(clicked()), this, SIGNAL(sigChangeViewLayout()));
	connect(axial_btn, SIGNAL(clicked()), this, SIGNAL(sigChangeAxialLayout()));
	connect(sagittal_btn, SIGNAL(clicked()), this, SIGNAL(sigChangeSagittalLayout()));
	connect(coronal_btn, SIGNAL(clicked()), this, SIGNAL(sigChangeCoronalLayout()));

	tab_layout->addWidget(axial_btn);
	tab_layout->addWidget(sagittal_btn);
	tab_layout->addWidget(coronal_btn);
	tab_layout->addWidget(close);


	connect(m_otf_view, &OTFWidget::sigOTFClicked, [this]() {this->slotOTFClicked(); });
	connect(m_otf_view, &OTFWidget::sigOTFReleased, [this]() {this->slotOTFReleased(); });
	connect(m_otf_view, &OTFWidget::sigChangeTF, [this](QList<OTFPolygon*> polyList, int offset) {this->slotChangeTF(polyList, offset); });
	connect(m_otf_view, &OTFWidget::sigChangeColor, [this](QList<OTFColor> colorList) {this->slotChangeColor(colorList); });
	connect(m_otf_view, SIGNAL(sigSetScrollHandDrag(void)), m_otf_window, SLOT(slotSetScrollHandDrag(void)));
	connect(m_otf_view, SIGNAL(sigOTFPresetLoad(void)), this, SLOT(slotShowOTFPreset(void)));
	connect(m_otf_view, &OTFWidget::sigChangeAll, [this](QList<OTFColor> colorList, QList<OTFPolygon*> polyList, int offset) {this->slotChangeAll(colorList, polyList, offset); });
	connect(m_otf_window, SIGNAL(sigLeave()), m_otf_view, SLOT(slotLeave()));
	connect(m_otf_window, SIGNAL(sigViewPortChanged(float)), m_otf_view, SLOT(slotViewPortChanged(float)));

}

void ViewLayout::setViewLayout(QLayout* layout) {
	QVBoxLayout* slots_layout = (QVBoxLayout*)this->layout();

	if (m_view_layout) {
		slots_layout->removeItem(m_view_layout);
		SAFE_DELETE_OBJECT(m_view_layout);
	}

	m_view_layout = layout;
	slots_layout->addLayout(m_view_layout);
}

void ViewLayout::resetOTF(VolumePacket volume) {
	
	//if (m_otf_view) {
	//	SAFE_DELETE_OBJECT(m_otf_view);
	//}
	//
	//if (m_otf_window) {
	//	SAFE_DELETE_OBJECT(m_otf_window);
	//}

	m_otf_view->initOTF(volume.MinVal, volume.MaxVal, volume.MinVal);



}

void ViewLayout::setMPRDVRLayout(QWidget* view1, QWidget* view2, QWidget* view3, QWidget* view4, int l) {
	if (l != 100) {
		tab->hide();
		view1->show();
		view2->show();
		view3->show();
		view4->show();
	}

	m_otf_window->hide();

	QVBoxLayout* right_layout = new QVBoxLayout;
	right_layout->addWidget(view3, l);
	right_layout->addWidget(view4, 100 - l);

	QVBoxLayout* left_layout = new QVBoxLayout;
	left_layout->addWidget(view1, l);
	left_layout->addWidget(view2, 100 - l);

	QHBoxLayout* view_layout = new QHBoxLayout;
	view_layout->addLayout(left_layout, l);
	view_layout->addLayout(right_layout, 100 - l);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(tab);
	layout->addLayout(view_layout);

	setViewLayout(layout);
}

void ViewLayout::setMPRLayoutVH(QWidget* view1, QWidget* view2, QWidget* view3) {
	QHBoxLayout* bottom_layout = new QHBoxLayout;
	bottom_layout->addWidget(view2);
	bottom_layout->addWidget(view3);

	QVBoxLayout* main_layout = new QVBoxLayout;
	main_layout->addWidget(view1);
	main_layout->addLayout(bottom_layout);

	setViewLayout(main_layout);
}

void ViewLayout::setMPRLayoutHV(QWidget* view1, QWidget* view2, QWidget* view3, int l) {
	if (l != 100) {
		tab->hide();
		view1->show();
		view2->show();
		view3->show();
	}

	QVBoxLayout* right_layout = new QVBoxLayout;
	right_layout->addWidget(view2, 50);
	right_layout->addWidget(view3, 50);

	QVBoxLayout* left_layout = new QVBoxLayout;
	left_layout->addWidget(view1);

	QHBoxLayout* view_layout = new QHBoxLayout;
	view_layout->addLayout(left_layout, l);
	view_layout->addLayout(right_layout, 100 - l);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(tab);
	layout->addLayout(view_layout);

	setViewLayout(layout);
}

void ViewLayout::setViewFullScreen(QWidget* view1, QWidget* view2, QWidget* view3, QWidget* view4, SliceType slice_type) {
	switch (slice_type) {
	case AXIAL_SLICE:
		setMPRDVRLayout(view1, view2, view3, view4, 100);
		view1->show();
		view2->hide();
		view3->hide();
		view4->hide();
		tab->show();
		axial_btn->hide();
		sagittal_btn->show();
		coronal_btn->show();
		break;
	case SAGITTAL_SLICE:
		setMPRDVRLayout(view3, view1, view2, view4, 100);
		view1->hide();
		view2->hide();
		view3->show();
		view4->hide();
		tab->show();
		axial_btn->show();
		sagittal_btn->hide();
		coronal_btn->show();
		break;
	case CORONAL_SLICE:
		setMPRDVRLayout(view2, view3, view1, view4, 100);
		view1->hide();
		view2->show();
		view3->hide();
		view4->hide();
		tab->show();
		axial_btn->show();
		sagittal_btn->show();
		coronal_btn->hide();
		break;
	}
}

void ViewLayout::setViewFullScreen(QWidget* view1, QWidget* view2, QWidget* view3, SliceType slice_type) {
	switch (slice_type) {
	case AXIAL_SLICE:
		setMPRLayoutHV(view1, view2, view3, 100);
		view1->show();
		view2->hide();
		view3->hide();
		tab->show();
		axial_btn->hide();
		sagittal_btn->show();
		coronal_btn->show();
		break;
	case SAGITTAL_SLICE:
		setMPRLayoutHV(view3, view1, view2, 100);
		view1->hide();
		view2->hide();
		view3->show();
		tab->show();
		axial_btn->show();
		sagittal_btn->hide();
		coronal_btn->show();
		break;
	case CORONAL_SLICE:
		setMPRLayoutHV(view2, view3, view1, 100);
		view1->hide();
		view2->show();
		view3->hide();
		tab->show();
		axial_btn->show();
		sagittal_btn->show();
		coronal_btn->hide();
		break;
	}
}

void ViewLayout::setViewFullScreen2(QWidget* view1, QWidget* view2, QWidget* view3, SliceType slice_type) {
	QHBoxLayout* layout = new QHBoxLayout;
	QTabWidget* main_tab = new QTabWidget;
	main_tab->setTabPosition(QTabWidget::North);

	QPushButton* close = new QPushButton;
	close->setIcon(QIcon("Images/Slice/out.png"));
	close->setIconSize(QSize(15, 15));

	main_tab->setCornerWidget(close, Qt::TopRightCorner);

	switch (slice_type) {
	case AXIAL_SLICE:
		main_tab->addTab(view1, "Axial");
		main_tab->addTab(view2, "Coronal");
		main_tab->addTab(view3, "Sagittal");
		break;
	case SAGITTAL_SLICE:
		main_tab->addTab(view3, "Sagittal");
		main_tab->addTab(view2, "Coronal");
		main_tab->addTab(view1, "Axial");
		tab->show();
		axial_btn->show();
		sagittal_btn->hide();
		coronal_btn->show();
		break;
	case CORONAL_SLICE:
		main_tab->addTab(view2, "Coronal");
		main_tab->addTab(view1, "Axial");
		main_tab->addTab(view3, "Sagittal");
		break;
	}

	connect(close, SIGNAL(clicked()), this, SIGNAL(sigChangeViewLayout()));
	layout->addWidget(main_tab);

	setViewLayout(layout);
}

bool ViewLayout::setCPRLayout(QWidget* view1, QWidget* view2, QWidget* view3, SliceType slice_type, int l) {
	if (view1 == nullptr || view2 == nullptr || view3 == nullptr) return false;

	if (l != 100) {
		tab->show();
		view1->show();
		view2->show();
		view3->show();
	}

	switch (slice_type) {
	case AXIAL_SLICE:
		axial_btn->hide();
		sagittal_btn->show();
		coronal_btn->show();
		break;
	case SAGITTAL_SLICE:
		axial_btn->show();
		sagittal_btn->hide();
		coronal_btn->show();
		break;
	case CORONAL_SLICE:
		axial_btn->show();
		sagittal_btn->show();
		coronal_btn->hide();
		break;
	}


	m_otf_window->hide();

	QVBoxLayout* right_layout = new QVBoxLayout;
	right_layout->addWidget(view3, l);
	right_layout->addWidget(view2, 100 - l);
	QVBoxLayout* left_layout = new QVBoxLayout;
	left_layout->addWidget(view1, l);

	QHBoxLayout* view_layout = new QHBoxLayout;
	view_layout->addLayout(left_layout, l);
	view_layout->addLayout(right_layout, 100 - l);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(tab);
	layout->addLayout(view_layout);

	setViewLayout(layout);
	return true;
}


void ViewLayout::setOTFView(QWidget* view1, QWidget* view2, QWidget* view3, QWidget* view4) {
	view1->show();
	view2->show();
	view3->show();
	view4->show();
	m_otf_window->show();
	tab->show();
	axial_btn->hide();
	sagittal_btn->hide();
	coronal_btn->hide();

	QVBoxLayout* mpr_layout = new QVBoxLayout;
	mpr_layout->addWidget(view1, 30);
	mpr_layout->addWidget(view2, 30);
	mpr_layout->addWidget(view3, 30);

	QHBoxLayout* view_layout = new QHBoxLayout;
	view_layout->addWidget(view4, 65);
	view_layout->addLayout(mpr_layout, 35);


	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(tab);
	layout->addLayout(view_layout, 70);
	layout->addWidget(m_otf_window, 20);

	setViewLayout(layout);
}

void ViewLayout::slotChangeTF(QList<OTFPolygon*> polyList, int offset) {
	emit sigChangeTF(polyList, offset);
}

void ViewLayout::slotChangeAll(QList<OTFColor> colorList, QList<OTFPolygon*> polyList, int offset) {
	emit sigChangeAll(colorList, polyList, offset);
}

void ViewLayout::slotOTFClicked() {
	emit sigOTFClicked();
}

void ViewLayout::slotOTFReleased() {
	emit sigOTFReleased();
}

void ViewLayout::slotChangeColor(QList<OTFColor> colorList) {
	emit sigChangeColor(colorList);
}

void ViewLayout::slotShowOTFPreset(void)
{
	if (m_pOTFPresetViewer) {
		disconnect(m_pOTFPresetViewer, SIGNAL(sigSelected(int)), m_otf_view, SLOT(slotSelected(int)));
		//disconnect(m_pOTFPresetViewer, SIGNAL(sigClosed(void)), this, SIGNAL(sigOTFPresetClosed(void)));
		SAFE_DELETE_OBJECT(m_pOTFPresetViewer);
	}
	if (!m_pOTFPresetViewer) {
		m_pOTFPresetViewer = new OTFPresetViewer(m_otf_view->getTFPath(), this->mapToGlobal(QPoint(0, 0)));
		connect(m_pOTFPresetViewer, SIGNAL(sigSelected(int)), m_otf_view, SLOT(slotSelected(int)));
		//connect(m_pOTFPresetViewer, SIGNAL(sigClosed(void)), this, SIGNAL(sigOTFPresetClosed(void)));
	}
	int nPrgWidth = 1920;
	int nPrgHeight = 1370;
	
	QPoint ptPos(231, nPrgHeight - m_pOTFPresetViewer->height() - 250);

	m_pOTFPresetViewer->move(ptPos);
	m_pOTFPresetViewer->show();
	//if (!m_pOtfView->isVisible())
	//	emit sigSyncShowOTF(false);
	//emit sigSyncShowOTFPreset(true);
}