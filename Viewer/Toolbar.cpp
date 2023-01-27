#include "Toolbar.h"

#include <QTextEdit>
#include <QLabel>

MainToolBar::MainToolBar(QWidget* parent) : QToolBar(parent) {

	// Widgets for Mask.
	// Add MaskLayerWidget which can manage mask layers

	this->setContextMenuPolicy(Qt::PreventContextMenu);

	m_btn_back_to_welcome = new QPushButton("Back", this);
	addWidget(m_btn_back_to_welcome);

	m_mask_layer_widget = new MaskLayerWidget(QSize(300, 200), this);
	addWidget(m_mask_layer_widget);

	m_function_widget = new FunctionWidget(QSize(400, 300), this);
	addWidget(m_function_widget);

	connect(m_btn_back_to_welcome, SIGNAL(clicked()), this, SIGNAL(sigReqBackToWelcome()));

	connect(m_mask_layer_widget, &MaskLayerWidget::sigReqLoadMask, this, &MainToolBar::sigReqLoadMask);
	connect(m_mask_layer_widget, &MaskLayerWidget::sigReqExportMask, this, &MainToolBar::sigReqExportMask);
	connect(m_mask_layer_widget, &MaskLayerWidget::sigReqAddMask, this, &MainToolBar::sigReqAddMask);

	//connect(m_mask_layer_widget, &MaskLayerWidget::sigReqChangeCurMaskLabel, this, &MainToolBar::sigReqChangeCurMaskLabel);
	//connect(m_mask_layer_widget, &MaskLayerWidget::sigReqEraseMask, this, &MainToolBar::sigReqEraseMask);
	connect(m_mask_layer_widget, &MaskLayerWidget::sigReqChangeMaskVisible, this, &MainToolBar::sigReqChangeMaskVisible);
	connect(m_mask_layer_widget, &MaskLayerWidget::sigReqChangeMaskOpacity, this, &MainToolBar::sigReqChangeMaskOpacity);
	connect(m_mask_layer_widget, &MaskLayerWidget::sigReqChangeMaskColor, this, &MainToolBar::sigReqChangeMaskColor);

	connect(m_function_widget, SIGNAL(sigThicknessChanged(SliceType, int)), this, SIGNAL(sigThicknessChanged(SliceType, int)));
	connect(m_function_widget, SIGNAL(sigFunctionChanged(int, FunctionType)), this, SIGNAL(sigFunctionChanged(int, FunctionType)));
	connect(m_function_widget, SIGNAL(sigImageTypeChanged(int)), this, SIGNAL(sigImageTypeChanged(int)));
	//connect(m_function_widget, SIGNAL(sigChangeTF(QList<OTFPolygon*>)), this, SIGNAL(sigChangeTF(QList<OTFPolygon*>)));
	//connect(m_function_widget, SIGNAL(sigChangeColor(QList<OTFColor>)), this, SIGNAL(sigChangeColor(QList<OTFColor>)));
	//connect(m_function_widget, SIGNAL(sigOTFClicked()), this, SIGNAL(sigOTFClicked()));
	//connect(m_function_widget, SIGNAL(sigOTFReleased()), this, SIGNAL(sigOTFReleased()));
	connect(m_function_widget, SIGNAL(sigShaderStatus(int)), this, SIGNAL(sigShaderStatus(int)));
	connect(m_function_widget, SIGNAL(sigSliderDragged(float, float)), this, SIGNAL(sigSliderDragged(float, float)));
	connect(m_function_widget, SIGNAL(sigSliderClicked()), this, SIGNAL(sigSliderClicked()));
	connect(m_function_widget, SIGNAL(sigSliderReleased()), this, SIGNAL(sigSliderReleased()));
}

MainToolBar::~MainToolBar() {
	SAFE_DELETE_OBJECT(m_mask_layer_widget);
	SAFE_DELETE_OBJECT(m_btn_back_to_welcome);
}

void MainToolBar::resetMaskLayer() {
	m_mask_layer_widget->resetMaskLayer();
}

// QT Slots
void MainToolBar::slotAddedMask(int idx, MaskLabel label) {
	m_mask_layer_widget->addMaskLayer(idx, label);
}

/*
void MainToolBar::slotChangeCurMaskLabel(int cur_idx) {
	m_mask_layer_widget->changeCurMaskLabel(cur_idx);
}
*/

void MainToolBar::slotChangeMaskVisible(int cur_idx, bool is_visible) {
	m_mask_layer_widget->changeMaskVisible(cur_idx, is_visible);
}

void MainToolBar::toggleButton(FunctionType type) {
	m_function_widget->handleToggleButton(type);
}