#include "MaskLayerWidget.h"

#include <QLabel>
#include <QLineEdit>

MaskLayerWidget::MaskLayerWidget(QSize size, QWidget* parent):QWidget(parent) {

	this->setMinimumSize(size);
	this->setVisible(true);

	// Set layout of current widget.
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setAlignment(Qt::AlignTop);
	this->setLayout(layout);

	m_group_box = new QGroupBox(tr("Mask"));
	QVBoxLayout* vbox = new QVBoxLayout();
	m_btn_mask_import = new QPushButton("Import Mask", this);
	m_btn_mask_export = new QPushButton("Export Mask", this);
	vbox->addWidget(m_btn_mask_import);
	vbox->addWidget(m_btn_mask_export);
	m_btn_mask_export->hide();
	//m_btn_mask_load->setEnabled(false);
	m_list_layout = new QHBoxLayout();
	m_list_layout->setMargin(0);
	m_list_layout->setSpacing(0);
	m_list_layout->setAlignment(Qt::AlignTop);

	vbox->addLayout(m_list_layout);

	/*
	m_btn_mask_add = new QPushButton("Add", this);
	m_btn_mask_add->setEnabled(false);
	vbox->addWidget(m_btn_mask_add);
	*/

	QHBoxLayout* op_box = new QHBoxLayout();
	QLabel* op_label = new QLabel("Opacity");
	op_box->addWidget(op_label);

	m_slider_opacity = new QSlider(Qt::Horizontal, this);
	m_slider_opacity->setRange(0, 10);
	m_slider_opacity->setTickInterval(1);
	m_slider_opacity->setTickPosition(QSlider::TicksBelow);
	m_slider_opacity->setValue(5);
	m_slider_opacity->hide();
	op_box->addWidget(m_slider_opacity);

	vbox->addLayout(op_box);

	m_group_box->setLayout(vbox);

	layout->addWidget(m_group_box);

	connect(m_btn_mask_import, &QPushButton::clicked, this, &MaskLayerWidget::sigReqLoadMask);
	connect(m_btn_mask_export, &QPushButton::clicked, this, &MaskLayerWidget::sigReqExportMask);
	/*
	connect(m_btn_mask_add, &QPushButton::clicked, [=]() {
		emit sigReqAddMask(m_total_idx);
		});
	*/
	connect(m_slider_opacity, &QSlider::sliderPressed, [=]() {
		emit sigReqChangeMaskOpacity(-1);
		});
	connect(m_slider_opacity, &QSlider::valueChanged, [=](int v) {
		emit sigReqChangeMaskOpacity(v);
	});
	connect(m_slider_opacity, &QSlider::sliderReleased, [=]() {
		emit sigReqChangeMaskOpacity(-2);
		});
}

MaskLayerWidget::~MaskLayerWidget() {
	//SAFE_DELETE_OBJECT(m_btn_mask_add);
	SAFE_DELETE_OBJECT(m_list_layout);
}

void MaskLayerWidget::resetMaskLayer() {
	QRegularExpression exp("list-");
	QList<QWidget*> widgets = this->findChildren<QWidget*>(exp);
	for (int i = 0; i < widgets.length(); i++) {
		m_list_layout->removeWidget(widgets[i]);
		SAFE_DELETE_OBJECT(widgets[i]);
	}
	m_slider_opacity->hide();
}

void MaskLayerWidget::addMaskLayer(int idx, MaskLabel label) {
	// Add label data to label table.
	m_slider_opacity->show();

	m_labels[idx] = label;
	m_total_idx++;

	QHBoxLayout* list_layout = (QHBoxLayout*)m_list_layout->layout();

	QWidget* layer = new QWidget(this);
	layer->setObjectName("list-" + QString(idx));
	QHBoxLayout* layer_layout = new QHBoxLayout();
	layer_layout->setMargin(0);
	layer_layout->setSpacing(0);

	//QLabel* color_box = new QLabel(layer);
	//color_box->setFixedSize(QSize(16, 16));	// Make square
	//color_box->setFrameStyle(QFrame::Box);	// Draw a border
	QString style_sheet = "background-color: ";
	style_sheet += QString(label.color.c_str()) + ";";
	//color_box->setStyleSheet(style_sheet);
	//layer_layout->addWidget(color_box);

	//QLineEdit* layer_name = new QLineEdit(layer);
	//layer_name->setObjectName("Layer-" + QString(idx));
	//layer_name->setFixedHeight(16);
	//layer_name->setFixedWidth(300);
	//layer_name->setText(label.name.c_str());
	//layer_name->setStyleSheet("QLineEdit { border: none }");
	//layer_layout->addWidget(layer_name);

	/*
	QPushButton* btn_edit_layer = new QPushButton(layer);
	btn_edit_layer->setObjectName("Btn1-" + QString(idx));
	btn_edit_layer->setIcon(QIcon("Images/Mask/icon_edit1.png"));
	layer_layout->addWidget(btn_edit_layer, 1, Qt::AlignRight);
	*/
	layer->setFixedHeight(16);
	layer->setFixedWidth(150);

	QPushButton* btn_color_layer = new QPushButton(layer);
	btn_color_layer->setObjectName("Btn1-" + QString(idx));
	btn_color_layer->setStyleSheet(style_sheet);
	btn_color_layer->setText(label.name.c_str());
	QPushButton* btn_show_layer = new QPushButton(layer);
	btn_show_layer->setObjectName("Btn2-" + QString(idx));
	btn_show_layer->setIcon(QIcon("Images/Mask/show.png"));
	layer_layout->addWidget(btn_color_layer, 8);
	layer_layout->addWidget(btn_show_layer, 2);

	/*
	QPushButton* btn_op_layer = new QPushButton(layer);
	btn_op_layer->setObjectName("Btn3-" + QString(idx));
	btn_op_layer->setIcon(QIcon("Images/Mask/opacity.png"));
	btn_op_layer->setVisible(false);
	layer_layout->addWidget(btn_op_layer, 1, Qt::AlignRight);
	*/

	/*
	QPushButton* btn_reset_layer = new QPushButton(layer);
	btn_reset_layer->setObjectName("Btn4-" + QString(idx));
	btn_reset_layer->setIcon(QIcon("Images/Mask/reset.png"));
	layer_layout->addWidget(btn_reset_layer, 1, Qt::AlignRight);
	*/

	/*
	QPushButton* btn_remove_layer = new QPushButton;
	btn_remove_layer->setIcon(QIcon("icon/icon_delete.png"));
	layer_layout->addWidget(btn_remove_layer, 1, Qt::AlignRight);
	*/

	layer->setLayout(layer_layout);
	list_layout->addWidget(layer, 0, Qt::AlignTop);

	// Button signals connection
	/*
	connect(btn_edit_layer, &QPushButton::clicked, [=]() {
		emit sigReqChangeCurMaskLabel(idx);
	});
	*/

	connect(btn_show_layer, &QPushButton::clicked, [=]() {
		emit sigReqChangeMaskVisible(idx, !m_labels[idx].is_visible);
	});
	connect(btn_color_layer, &QPushButton::clicked, [=]() {
		QColor color = QColorDialog::getColor();

		QString style_sheet = "background-color: ";
		style_sheet += color.name() + ";";
		btn_color_layer->setStyleSheet(style_sheet);

		emit sigReqChangeMaskColor(idx, color.name().toStdString());
	});

	/*
	connect(btn_reset_layer, &QPushButton::clicked, [=]() {
		// Get selected label name
		std::string name = layer_name->text().toStdString();
		emit sigReqEraseMask(idx, name);
	});
	*/

	// Change cur label to novel one.
	//emit sigReqChangeCurMaskLabel(idx);
}

/*
void MaskLayerWidget::changeCurMaskLabel(int cur_idx) {
	// Remove highlight of other labels
	QLineEdit* prev_layer_name = this->findChild<QLineEdit*>("Layer-" + QString(m_cur_label_idx));
	QPushButton* prev_btn1 = this->findChild<QPushButton*>("Btn1-" + QString(m_cur_label_idx));

	if (prev_layer_name) {
		QFont font = prev_layer_name->font();
		font.setWeight(QFont::Normal);
		prev_layer_name->setFont(font);

		prev_btn1->setIcon(QIcon("Images/Mask/icon_edit1.png"));
	}

	// Set current label index
	m_cur_label_idx = cur_idx;

	// Highlight current label
	QLineEdit* cur_layer_name = this->findChild<QLineEdit*>("Layer-" + QString(m_cur_label_idx));
	QFont cur_font = cur_layer_name->font();
	cur_font.setWeight(QFont::Bold);
	cur_layer_name->setFont(cur_font);

	// Highlight edit icon
	QPushButton* cur_btn1 = this->findChild<QPushButton*>("Btn1-" + QString(m_cur_label_idx));
	cur_btn1->setIcon(QIcon("Images/Mask/icon_edit2.png"));
}
*/

void MaskLayerWidget::changeMaskVisible(int cur_idx, bool is_visible) {
	QPushButton* btn = this->findChild<QPushButton*>("Btn2-" + QString(cur_idx));
	m_labels[cur_idx].is_visible = is_visible;
	if (is_visible) {
		btn->setIcon(QIcon("Images/Mask/show.png"));
	}
	else {
		btn->setIcon(QIcon("Images/Mask/hide.png"));
	}
}
