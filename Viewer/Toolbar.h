#pragma once

#include <QToolbar>
#include <QLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>
#include <QString>

#include "MaskLayerWidget.h"
#include "FunctionWidget.h"
#include "build_global_opt.h"

class MainToolBar : public QToolBar {
	Q_OBJECT
signals:
	void sigThicknessChanged(SliceType, int);
	void sigReqBackToWelcome();
	void sigReqShowInfo();
	void sigFunctionChanged(int, FunctionType);
	void sigImageTypeChanged(int);
	void sigChangeTF(QList<OTFPolygon*>);
	void sigChangeColor(QList<OTFColor>);
	void sigOTFClicked();
	void sigOTFReleased();
	void sigShaderStatus(int);
	void sigSliderDragged(float, float);
	void sigSliderClicked();
	void sigSliderReleased();
	void sigReqLoadMask();
	void sigReqExportMask();
	void sigReqAddMask(int);
	//void sigReqSaveMask();
	void sigReqChangeMaskVisible(int idx, bool);
	void sigReqChangeMaskOpacity(int val);
	void sigReqChangeMaskColor(int idx, std::string);
	//void sigReqChangeCurMaskLabel(int idx);
	//void sigReqEraseMask(int idx, std::string layer_name);

public slots:
	void slotAddedMask(int idx, MaskLabel label);
	//void slotChangeCurMaskLabel(int cur_idx);

public:
	MainToolBar(QWidget* parent=nullptr);
	~MainToolBar();

	void slotChangeMaskVisible(int cur_idx, bool is_visible);
	void resetMaskLayer();

	void toggleButton(FunctionType type);

private:
	QPushButton* m_btn_back_to_welcome;
	MaskLayerWidget *m_mask_layer_widget;
	FunctionWidget* m_function_widget;
};