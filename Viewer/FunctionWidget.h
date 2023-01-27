#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QtCharts>
#include "OTFWidget.h"
#include "../ViewerManager/global.h"

class FunctionWidget : public QWidget
{
	Q_OBJECT

public:
	FunctionWidget(QSize size, QWidget* parent);

	FunctionType getFunctionType() { return m_function_type; }


signals:
	void sigThicknessChanged(SliceType, int);
	void sigFunctionChanged(int, FunctionType);
	void sigImageTypeChanged(int);
	void sigShaderStatus(int);

	void sigSliderDragged(float, float);
	void sigSliderClicked();
	void sigSliderReleased();

public slots:
	//void handleProjectionToggleButton(FunctionType);
	void handleToggleButton(FunctionType);
	void handlePushButton(FunctionType);


private:
	QPushButton* button_inverse;
	QRadioButton* thickness_avg, * thickness_min, * thickness_max;
	QSpinBox* axial_thickness, * coronal_thickness, * sagittal_thickness;

	QPushButton* button_reset;
	QPushButton* button_sculpt;
	QPushButton* button_crop;
	QPushButton* button_voi;

	QPushButton* button_A;
	QPushButton* button_P;
	QPushButton* button_L;
	QPushButton* button_R;
	QPushButton* button_H;
	QPushButton* button_F;

	QRadioButton* proj_Default, *proj_MaxIP, *proj_MinIP, *proj_RaySum, *proj_OTF;

	QRadioButton* res_mpr_256, *res_mpr_512, *res_mpr_1024;
	QRadioButton* res_vr_256, * res_vr_512, * res_vr_1024;
	QRadioButton* res_ds_128, * res_ds_192, * res_ds_256, * res_ds_384, * res_ds_512, * ds_off;
	QRadioButton* res_compress_25, * res_compress_50, * res_compress_75, * res_compress_100;
	QRadioButton* byteArray_On, * jpeg_On;
	QRadioButton* m_shader_on, * m_shader_off;
	QSlider* m_specular;
	QSlider* m_brightness;

	QGraphicsView* otf_view;

	FunctionType m_function_type;

	QGroupBox* _addVRFunctionGroup();
	QGroupBox* _addMPRFunctionGroup();
	QGroupBox* OTFBox;
	QGroupBox* compressBox;
	QGroupBox* _addResolutionGroup();

	QPushButton* _addButton(QHBoxLayout*, QString name);
	QGraphicsView* _addOtf(int min, int max, QHBoxLayout*, QString name);
	OTFWidget* otf_widget = new OTFWidget();


	void _initButtonIcon();
	void _resetButton();

};