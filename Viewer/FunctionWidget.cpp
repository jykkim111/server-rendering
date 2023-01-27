#include "FunctionWidget.h"

#include <QLayout>
#include <QDebug>
#include <QPainter>

FunctionWidget::FunctionWidget(QSize size, QWidget* parent) : QWidget(parent) {

	this->setMinimumSize(size);
	this->setMaximumWidth(500);
	this->setVisible(true);
	QVBoxLayout* layout = new QVBoxLayout();

	this->setLayout(layout);

	layout->addWidget(_addResolutionGroup());
	layout->addWidget(_addMPRFunctionGroup());
	layout->addWidget(_addVRFunctionGroup());

	_initButtonIcon();

	//button_slice_interpolation->setEnabled(false);
	//button_measure->setEnabled(false);
	//button_edge->setEnabled(false);

	connect(button_inverse, &QPushButton::clicked, [this]() {this->handleToggleButton(MPR_INVERSE); });
	connect(thickness_avg, &QPushButton::clicked, [this]() {this->handlePushButton(MPR_THICKNESS_AVG); });
	connect(thickness_min, &QPushButton::clicked, [this]() {this->handlePushButton(MPR_THICKNESS_MIN); });
	connect(thickness_max, &QPushButton::clicked, [this]() {this->handlePushButton(MPR_THICKNESS_MAX); });
	connect(axial_thickness, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](int i) {emit sigThicknessChanged(AXIAL_SLICE, i); });
	connect(coronal_thickness, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](int i) {emit sigThicknessChanged(CORONAL_SLICE, i);});
	connect(sagittal_thickness, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](int i) {emit sigThicknessChanged(SAGITTAL_SLICE, i);});


	connect(button_sculpt, &QPushButton::clicked, [this]() {this->handleToggleButton(DVR_SCULPT); });
	connect(button_crop, &QPushButton::clicked, [this]() {this->handleToggleButton(DVR_CROP); });
	connect(button_voi, &QPushButton::clicked, [this]() {this->handleToggleButton(DVR_VOI); });
	connect(proj_MaxIP, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_MAXIP); });
	connect(proj_MinIP, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_MINIP); });
	connect(proj_RaySum, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_RAYSUM); });
	connect(proj_OTF, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_OTF); });
	connect(button_reset, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_RESET); });

	connect(button_A, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_A); });
	connect(button_P, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_P); });
	connect(button_L, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_L); });
	connect(button_R, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_R); });
	connect(button_H, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_H); });
	connect(button_F, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_F); });
	

	connect(res_mpr_256, &QPushButton::clicked, [this]() {this->handlePushButton(MPR_256); });
	connect(res_mpr_512, &QPushButton::clicked, [this]() {this->handlePushButton(MPR_512); });
	connect(res_mpr_1024, &QPushButton::clicked, [this]() {this->handlePushButton(MPR_1024); });
	connect(res_vr_256, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_256); });
	connect(res_vr_512, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_512); });
	connect(res_vr_1024, &QPushButton::clicked, [this]() {this->handlePushButton(DVR_1024); });
	connect(res_ds_128, &QPushButton::clicked, [this]() {this->handlePushButton(DS_128); });
	connect(res_ds_192, &QPushButton::clicked, [this]() {this->handlePushButton(DS_192); });
	connect(res_ds_384, &QPushButton::clicked, [this]() {this->handlePushButton(DS_384); });
	connect(res_ds_256, &QPushButton::clicked, [this]() {this->handlePushButton(DS_256); });
	connect(res_ds_512, &QPushButton::clicked, [this]() {this->handlePushButton(DS_512); });
	connect(ds_off, &QPushButton::clicked, [this]() {this->handlePushButton(DS_OFF); });

	connect(jpeg_On, &QPushButton::clicked, [this]() {this->handlePushButton(JPEG_ON); });
	connect(byteArray_On, &QPushButton::clicked, [this]() {this->handlePushButton(BYTE_ON); });
	connect(res_compress_25, &QPushButton::clicked, [this]() {this->handlePushButton(C_25); });
	connect(res_compress_50, &QPushButton::clicked, [this]() {this->handlePushButton(C_50); });
	connect(res_compress_75, &QPushButton::clicked, [this]() {this->handlePushButton(C_75); });
	connect(res_compress_100, &QPushButton::clicked, [this]() {this->handlePushButton(C_100); });

	connect(m_shader_on, &QPushButton::clicked, [this]() {this->handlePushButton(SHADER_ON); });
	connect(m_shader_off, &QPushButton::clicked, [this]() {this->handlePushButton(SHADER_OFF); });

}

QGroupBox* FunctionWidget::_addResolutionGroup() {
	QGroupBox* groupBox = new QGroupBox(tr("Resolution"));
	//groupBox->setFixedHeight(300);
	QVBoxLayout* layout = new QVBoxLayout();
	QGroupBox* groupBox_1 = new QGroupBox(tr("MPR"));
	QHBoxLayout* layout_1 = new QHBoxLayout();
	QGroupBox* groupBox_2 = new QGroupBox(tr("VR"));
	QHBoxLayout* layout_2 = new QHBoxLayout();
	QGroupBox* groupBox_3 = new QGroupBox(tr("Downsample"));
	QHBoxLayout* layout_3 = new QHBoxLayout();
	QGroupBox* groupBox_5 = new QGroupBox(tr("Image_type"));
	QHBoxLayout* layout_5 = new QHBoxLayout();
	compressBox = new QGroupBox(tr("Compression"));
	QHBoxLayout* layout_4 = new QHBoxLayout();
	

	res_mpr_256 = new QRadioButton(tr("256"));
	res_mpr_512 = new QRadioButton(tr("512"));
	res_mpr_1024 = new QRadioButton(tr("1024"));

	res_mpr_1024->setChecked(true);

	layout_1->addWidget(res_mpr_256);
	layout_1->addWidget(res_mpr_512);
	layout_1->addWidget(res_mpr_1024);
	groupBox_1->setLayout(layout_1);

	res_vr_256 = new QRadioButton(tr("256"));
	res_vr_512 = new QRadioButton(tr("512"));
	res_vr_1024 = new QRadioButton(tr("1024"));

	res_vr_1024->setChecked(true);

	layout_2->addWidget(res_vr_256);
	layout_2->addWidget(res_vr_512);
	layout_2->addWidget(res_vr_1024);
	groupBox_2->setLayout(layout_2);

	res_ds_128 = new QRadioButton(tr("128"));
	res_ds_256 = new QRadioButton(tr("256"));
	res_ds_192 = new QRadioButton(tr("192"));
	res_ds_384 = new QRadioButton(tr("384"));
	res_ds_512 = new QRadioButton(tr("512"));
	ds_off = new QRadioButton(tr("off"));

	res_ds_128->setChecked(true);

	layout_3->addWidget(res_ds_128);
	layout_3->addWidget(res_ds_192);
	layout_3->addWidget(res_ds_256);
	layout_3->addWidget(res_ds_384);
	layout_3->addWidget(res_ds_512);
	layout_3->addWidget(ds_off);
	groupBox_3->setLayout(layout_3);


	jpeg_On = new QRadioButton(tr("jpeg"));
	byteArray_On = new QRadioButton(tr("byte"));
	jpeg_On->setChecked(true);

	layout_5->addWidget(jpeg_On);
	layout_5->addWidget(byteArray_On);
	groupBox_5->setLayout(layout_5);

	res_compress_25 = new QRadioButton(tr("25"));
	res_compress_50 = new QRadioButton(tr("50"));
	res_compress_75 = new QRadioButton(tr("75"));
	res_compress_100 = new QRadioButton(tr("100"));

	res_compress_75->setChecked(true);

	layout_4->addWidget(res_compress_25);
	layout_4->addWidget(res_compress_50);
	layout_4->addWidget(res_compress_75);
	layout_4->addWidget(res_compress_100);
	compressBox->setLayout(layout_4);

	layout->addWidget(groupBox_1, Qt::AlignLeft);
	layout->addWidget(groupBox_2, Qt::AlignLeft);
	layout->addWidget(groupBox_3, Qt::AlignLeft); 
	layout->addWidget(groupBox_5, Qt::AlignLeft);
	layout->addWidget(compressBox, Qt::AlignLeft);
	groupBox->setLayout(layout);

	return groupBox;
}

QGroupBox* FunctionWidget::_addMPRFunctionGroup() {
	QGroupBox* groupBox = new QGroupBox(tr("MPR"));
	groupBox->setFixedHeight(150);
	QHBoxLayout* layout = new QHBoxLayout();
	button_inverse = _addButton(layout, "Inverse");

	QGroupBox* groupBox_1 = new QGroupBox(tr("Thickness"));
	QHBoxLayout* layout_1 = new QHBoxLayout();
	thickness_avg = new QRadioButton(tr("AvgIP"));
	thickness_min = new QRadioButton(tr("MinIP"));
	thickness_max = new QRadioButton(tr("MaxIP"));
	layout_1->addWidget(thickness_avg);
	layout_1->addWidget(thickness_min);
	layout_1->addWidget(thickness_max);
	thickness_avg->setChecked(true);
	QLabel* axial = new QLabel(tr("axial"));
	axial_thickness = new QSpinBox;
	axial_thickness->setRange(0, 50);
	axial_thickness->setSingleStep(1);
	axial_thickness->setValue(0);
	QLabel* coronal = new QLabel(tr("coronal"));
	coronal_thickness = new QSpinBox;
	coronal_thickness->setRange(0, 50);
	coronal_thickness->setSingleStep(1);
	coronal_thickness->setValue(0);
	QLabel* sagittal = new QLabel(tr("sagittal"));
	sagittal_thickness = new QSpinBox;
	sagittal_thickness->setRange(0, 50);
	sagittal_thickness->setSingleStep(1);
	sagittal_thickness->setValue(0);
	layout_1->addWidget(axial);
	layout_1->addWidget(axial_thickness);
	layout_1->addWidget(coronal);
	layout_1->addWidget(coronal_thickness);
	layout_1->addWidget(sagittal);
	layout_1->addWidget(sagittal_thickness);
	groupBox_1->setLayout(layout_1);

	layout->addWidget(groupBox_1, Qt::AlignLeft);
	groupBox->setLayout(layout);

	return groupBox;
}

QGroupBox* FunctionWidget::_addVRFunctionGroup() {
	QGroupBox* groupBox = new QGroupBox(tr("VR"));
	//groupBox->setFixedHeight(700);
	QVBoxLayout* layout = new QVBoxLayout();
	QHBoxLayout* layout_1 = new QHBoxLayout();
	QHBoxLayout* layout_2 = new QHBoxLayout();
	QGroupBox* groupBox_2 = new QGroupBox(tr("Projections"));
	//groupBox_2->setFixedHeight(50);
	QHBoxLayout* layout_3 = new QHBoxLayout();
	OTFBox = new QGroupBox(tr("OTF-View"));
	//OTFBox->setFixedHeight(450);
	QGroupBox* mShader_groupBox = new QGroupBox(tr("Shader"));
	QVBoxLayout* layout2 = new QVBoxLayout();
	QHBoxLayout* layout_4 = new QHBoxLayout();
	QHBoxLayout* layout_5 = new QHBoxLayout();
	QHBoxLayout* layout_6 = new QHBoxLayout();
	QGroupBox* spec_sliderBox = new QGroupBox(tr("specular"));
	QGroupBox* brightness_sliderBox = new QGroupBox(tr("brightness"));
	QHBoxLayout* layout_7 = new QHBoxLayout();
	QHBoxLayout* layout_8 = new QHBoxLayout();
	QPalette pal = palette();


	button_reset = _addButton(layout_1, "Undo");
	button_sculpt = _addButton(layout_1, "Sculpt");
	button_crop = _addButton(layout_1, "Crop");
	button_voi = _addButton(layout_1, "VOI");

	button_A = _addButton(layout_2, "A");
	button_P = _addButton(layout_2, "P");
	button_L = _addButton(layout_2, "L");
	button_R = _addButton(layout_2, "R");
	button_H = _addButton(layout_2, "H");
	button_F = _addButton(layout_2, "F");

	proj_OTF = new QRadioButton(tr("OTF"));
	proj_MaxIP = new QRadioButton(tr("Max_IP"));
	proj_MinIP = new QRadioButton(tr("Min_IP"));
	proj_RaySum = new QRadioButton(tr("Ray Sum"));


	proj_OTF->setChecked(true);

	layout_3->addWidget(proj_OTF);
	layout_3->addWidget(proj_MaxIP);
	layout_3->addWidget(proj_MinIP);
	layout_3->addWidget(proj_RaySum);

	groupBox_2->setLayout(layout_3);


	//otf_view = _addOtf(-1024, 1596, layout_5, "otf");
	//otf_widget->initOTF(1596 + 1024, 0);
	//QGraphicsView* otf_view = new QGraphicsView(otf_widget);
	//layout_5->addWidget(otf_view);

	m_shader_on = new QRadioButton(tr("on"));
	m_shader_off = new QRadioButton(tr("off"));
	m_shader_on->setChecked(true);
	layout_6->addWidget(m_shader_on);
	layout_6->addWidget(m_shader_off);
	mShader_groupBox->setLayout(layout_6);

	m_specular = new QSlider(Qt::Horizontal);
	m_specular->setMinimum(0);
	m_specular->setMaximum(10);
	m_specular->setSliderPosition(7);
	QLabel* specular_val = new QLabel("0.7");
	pal.setColor(specular_val->foregroundRole(), Qt::black);
	specular_val->setPalette(pal);
	layout_7->addWidget(m_specular);
	layout_7->addWidget(specular_val);
	spec_sliderBox->setLayout(layout_7);

	m_brightness = new QSlider(Qt::Horizontal);
	m_brightness->setMinimum(0);
	m_brightness->setMaximum(10);
	m_brightness->setSliderPosition(3);
	QLabel* brightness_val = new QLabel("0.3");
	pal.setColor(brightness_val->foregroundRole(), Qt::black);
	brightness_val->setPalette(pal);
	layout_8->addWidget(m_brightness);
	layout_8->addWidget(brightness_val);
	brightness_sliderBox->setLayout(layout_8);


	connect(m_specular, &QSlider::valueChanged, [=](int v) {
		specular_val->setText(QString::number(v /10.0));
		if (m_specular->isSliderDown())
			emit sigSliderDragged(m_specular->value(), m_brightness->value());
		});
	connect(m_brightness, &QSlider::valueChanged, [=](int v) {
		brightness_val->setText(QString::number(v / 10.0));
		if (m_brightness->isSliderDown())
			emit sigSliderDragged(m_specular->value(), m_brightness->value());
		});

	connect(m_specular, &QSlider::sliderPressed, [=]() {
		emit sigSliderClicked();
		});
	connect(m_brightness, &QSlider::sliderPressed, [=]() {
		emit sigSliderClicked();
		});

	connect(m_specular, &QSlider::sliderReleased, [=]() {
		emit sigSliderReleased();
		});
	connect(m_brightness, &QSlider::sliderReleased, [=]() {
		emit sigSliderReleased();
		});


	layout->addLayout(layout_1, Qt::AlignLeft);
	layout->addLayout(layout_2, Qt::AlignLeft);

	//layout2->addLayout(layout_5, Qt::AlignLeft);
	layout2->addWidget(mShader_groupBox);
	layout2->addWidget(spec_sliderBox);
	layout2->addWidget(brightness_sliderBox);

	OTFBox->setLayout(layout2);
	layout->addWidget(groupBox_2, Qt::AlignLeft);
	layout->addWidget(OTFBox, Qt::AlignLeft);
	groupBox->setLayout(layout);
	
	//groupBox_3->setEnabled(false);

	return groupBox;
}




QPushButton* FunctionWidget::_addButton(QHBoxLayout* m_layout, QString name) {
	QVBoxLayout* boxlayout = new QVBoxLayout();
	QPushButton* new_button = new QPushButton;
	QLabel* textbox = new QLabel;

	new_button->setStyleSheet("text-align:center;");
	//new_button->setIconSize(QSize(10, 10));
	const QSize btnSize = QSize(60, 30);
	QPalette pal = new_button->palette();
	pal.setColor(QPalette::Button, QColor(Qt::white));
	new_button->setAutoFillBackground(true);
	new_button->setPalette(pal);
	new_button->setFixedSize(btnSize);
	new_button->setText(name);

	//boxlayout->addWidget(textbox);
	boxlayout->addWidget(new_button, Qt::AlignLeft);

	m_layout->addLayout(boxlayout, Qt::AlignLeft);

	return new_button;
}

QGraphicsView* FunctionWidget::_addOtf(int min, int max, QHBoxLayout* m_layout, QString name) {
	otf_widget->initOTF(min, max, min);
	QGraphicsView* otf_view = new QGraphicsView(otf_widget);

	return otf_view;
}


void FunctionWidget::_resetButton() {
	if (m_function_type == DVR_SCULPT) {
		button_sculpt->setFlat(false);
	}
	else if (m_function_type == DVR_CROP) {
		button_crop->setFlat(false);
	}
	else if (m_function_type == DVR_VOI) {
		button_voi->setFlat(false);
	}
	else
		_initButtonIcon();
}



void FunctionWidget::_initButtonIcon() {
	button_sculpt->setFlat(false);
	button_crop->setFlat(false);
	button_voi->setFlat(false);
}

/* ========================================================================== */
/*									QT Slots								  */
/* ========================================================================== */


void FunctionWidget::handleToggleButton(FunctionType type) {
	QPushButton* button;
	QIcon* on, * off;
	switch (type) {
	case MPR_INVERSE:
		button = button_inverse;
		break;
	case DVR_SCULPT:
		button = button_sculpt;
		break;
	case DVR_CROP:
		button = button_crop;
		break;
	case DVR_VOI:
		button = button_voi;
		break;
	}

	switch (type) {
	case MPR_INVERSE:
	case DVR_SCULPT:
	case DVR_CROP:
	case DVR_VOI:
	
		if (button->isFlat()) {
			button->setFlat(false);
			m_function_type = NONE;
			emit sigFunctionChanged(0, type);
		}
		else {
			_resetButton();
			button->setFlat(true);
			m_function_type = type;
			emit sigFunctionChanged(1, type);
		}
		break;
	}
}


void FunctionWidget::handlePushButton(FunctionType type) {
	switch (type) {
	case DVR_RESET:
		_resetButton();
		m_function_type = NONE;
		emit sigFunctionChanged(1, type);
		break;
	case DVR_A:
	case DVR_P:
	case DVR_L:
	case DVR_R:
	case DVR_H:
	case DVR_F:		
	case MPR_256:
	case MPR_512:
	case MPR_1024:
	case DVR_256:
	case DVR_512:
	case DVR_1024:
	case DS_128:
	case DS_192:
	case DS_384:
	case DS_256:
	case DS_512:
	case DS_OFF:
	case C_25:
	case C_50:
	case C_75:
	case C_100:
	case MPR_THICKNESS_AVG:
	case MPR_THICKNESS_MIN:
	case MPR_THICKNESS_MAX:
		m_function_type = NONE;
		emit sigFunctionChanged(1, type);
		break;
	case DVR_MAXIP:
		OTFBox->setEnabled(false);
		m_function_type = NONE;
		emit sigFunctionChanged(1, type);
		break;
	case DVR_MINIP:
		OTFBox->setEnabled(false);
		m_function_type = NONE;
		emit sigFunctionChanged(1, type);
		break;
	case DVR_RAYSUM:
		OTFBox->setEnabled(false);
		m_function_type = NONE;
		emit sigFunctionChanged(1, type);
		break;
	case DVR_OTF:
		OTFBox->setEnabled(true);
		m_function_type = NONE;
		emit sigFunctionChanged(1, type);
		break;
	case BYTE_ON:
		compressBox->setEnabled(false);
		emit sigImageTypeChanged(0);
		break;
	case JPEG_ON:
		compressBox->setEnabled(true);
		emit sigImageTypeChanged(1);
		break;
	case SHADER_ON:
		m_specular->setEnabled(true);
		m_brightness->setEnabled(true);
		emit sigShaderStatus(1);
		break;
	case SHADER_OFF:
		m_specular->setEnabled(false);
		m_brightness->setEnabled(false);
		emit sigShaderStatus(0);
		break;
	}
	

}
