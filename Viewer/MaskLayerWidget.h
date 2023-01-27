#pragma once

#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QRegularExpression>
#include <QColorDialog>

#include <string>

#include "../ViewerManager/global.h"
#include "build_global_opt.h"

class MaskLayerWidget : public QWidget {
	Q_OBJECT
signals:
	void sigReqLoadMask();
	void sigReqExportMask();
	void sigReqAddMask(int);
	//void sigReqChangeCurMaskLabel(int idx);
	void sigReqChangeMaskVisible(int idx, bool);
	void sigReqChangeMaskOpacity(int val);
	void sigReqChangeMaskColor(int idx, std::string);
	//void sigReqEraseMask(int idx, std::string layer_name);

public slots:

public:
	MaskLayerWidget(QSize size, QWidget* parent=nullptr);
	~MaskLayerWidget();

	void resetMaskLayer();
	void addMaskLayer(int idx, MaskLabel label);
	//void changeCurMaskLabel(int cur_idx);
	void changeMaskVisible(int cur_idx, bool is_visible);

private:
	int m_total_idx = 0;	// 0 means background label

	QGroupBox* m_group_box;

	QPushButton* m_btn_mask_import;
	QPushButton* m_btn_mask_export;
	QPushButton* m_btn_mask_add;
	QSlider* m_slider_opacity;
	QHBoxLayout* m_list_layout;

	// Tables for mask label info
	MaskLabel* m_labels = new MaskLabel[G_MAX_MASK_NUM + 1];
};