#include "SliceWidget.h"
#include <QLabel>
#include <QLayout>
#include <QDebug>
#include <QPainter>
#include <QTimer>

#include <algorithm>
#include <vector>
#include <iostream>


SliceWidget::SliceWidget(QWidget* parent, const char* name, SliceType slice_type) : QGraphicsView(parent) {
	// Set name
	this->setObjectName(name);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setRenderHint(QPainter::Antialiasing, true);

	QGraphicsScene* pScene = new QGraphicsScene(this);
	this->setScene(pScene);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setMinimumSize(150, 150);		// This make the windows can be shrinked.

	m_img = new QPixmap(512, 512);
	m_img->fill(QColor("black"));

	m_slice_type = slice_type;

	this->setMouseTracking(true);
	this->setBackgroundBrush(Qt::black);

	m_rendered_img = this->scene()->addPixmap(m_img->scaled(this->width(), this->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	this->scene()->setSceneRect(-20000, -20000, 40000, 40000);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_center_origin = QPoint(m_img->width() / 2, m_img->height() / 2);
	m_rendered_img->setPos(0, 0);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
	this->scale(1.0, 1.0);
	m_windowed_img = new QPixmap();

	

	std::cout << "created widget" << std::endl;

	if (strcmp(name, "Preview") != 0) {
		m_button_layout = new QVBoxLayout(this);
		// zooming
		QVBoxLayout* m_zoom_box = new QVBoxLayout;
		m_zoom_box->setAlignment(Qt::AlignRight | Qt::AlignTop);

		QPushButton* plus_button = new QPushButton;
		plus_button->setObjectName("btn_3");
		plus_button->setIcon(QIcon("Images/Slice/plus.png"));

		QPushButton* minus_button = new QPushButton;
		minus_button->setObjectName("btn_4");
		minus_button->setIcon(QIcon("Images/Slice/minus.png"));

		connect(plus_button, &QPushButton::clicked, [this] {emit sigScreenDragged(0, -0.05, LEFT_CLICK);});
		connect(minus_button, &QPushButton::clicked, [this] {emit sigScreenDragged(0, 0.05, LEFT_CLICK);});

		m_zoom_box->addWidget(plus_button);
		m_zoom_box->addWidget(minus_button);
		m_button_layout->addLayout(m_zoom_box, Qt::AlignRight | Qt::AlignTop);

		// slider
		QHBoxLayout* m_bottom_box = new QHBoxLayout;
		m_bottom_box->setAlignment(Qt::AlignBottom);

		QVBoxLayout* m_slider_box = new QVBoxLayout;
		m_slider_box->setAlignment(Qt::AlignBottom | Qt::AlignLeft);

		QHBoxLayout* wl_box = new QHBoxLayout;
		QLabel* wl_text = new QLabel("WL");
		wl_text->setMouseTracking(true);
		QPalette pal = palette();
		pal.setColor(wl_text->foregroundRole(), Qt::green);
		wl_text->setPalette(pal);
		m_wl_slider = new QSlider(Qt::Horizontal);
		m_wl_slider->setRange(0, 10);
		m_wl_slider->setTickInterval(20);
		m_wl_slider->setTickPosition(QSlider::TicksBelow);
		m_wl_slider->setValue(5);
		m_wl_slider->setMaximumWidth(180);
		QLabel* wl_val = new QLabel("5");
		pal.setColor(wl_val->foregroundRole(), Qt::green);
		wl_val->setPalette(pal);
		wl_val->setMouseTracking(true);
		wl_box->addWidget(wl_text);
		wl_box->addWidget(m_wl_slider);
		wl_box->addWidget(wl_val);

		QHBoxLayout* ww_box = new QHBoxLayout;
		QLabel* ww_text = new QLabel("WW");
		ww_text->setMouseTracking(true);
		pal.setColor(ww_text->foregroundRole(), Qt::green);
		ww_text->setPalette(pal);
		m_ww_slider = new QSlider(Qt::Horizontal);
		m_ww_slider->setRange(0, 10);
		m_ww_slider->setTickInterval(20);
		m_ww_slider->setTickPosition(QSlider::TicksBelow);
		m_ww_slider->setValue(5);
		m_ww_slider->setMaximumWidth(180);
		QLabel* ww_val = new QLabel("5");
		pal.setColor(ww_val->foregroundRole(), Qt::green);
		ww_val->setPalette(pal);
		ww_box->addWidget(ww_text);
		ww_box->addWidget(m_ww_slider);
		ww_box->addWidget(ww_val);

		m_slider_box->addLayout(wl_box);
		m_slider_box->addLayout(ww_box);

		connect(m_wl_slider, &QSlider::valueChanged, [=](int v) {
			wl_val->setText(QString::number(v));
			if(m_wl_slider->isSliderDown())
				emit sigScreenDragged(m_ww_slider->value(), m_wl_slider->value(), RIGHT_CLICK);
		});
		connect(m_ww_slider, &QSlider::valueChanged, [=](int v) {
			ww_val->setText(QString::number(v));
			if (m_ww_slider->isSliderDown())
				emit sigScreenDragged(m_ww_slider->value(), m_wl_slider->value(), RIGHT_CLICK);
		});

		connect(m_wl_slider, &QSlider::sliderPressed, [=]() {
			emit sigClicked(0, 0, LEFT_CLICK);
			});
		connect(m_ww_slider, &QSlider::sliderPressed, [=]() {
			emit sigClicked(0, 0, LEFT_CLICK);
			});

		connect(m_wl_slider, &QSlider::sliderReleased, [=]() {
			emit sigMouseReleased(0, 0, LEFT_CLICK);
			});
		connect(m_ww_slider, &QSlider::sliderReleased, [=]() {
			emit sigMouseReleased(0, 0, LEFT_CLICK);
			});

		m_bottom_box->addLayout(m_slider_box, Qt::AlignLeft);

		// full screen / guideline
		QHBoxLayout* m_others_box = new QHBoxLayout;
		m_others_box->setAlignment(Qt::AlignBottom | Qt::AlignRight);

		QPushButton* full_screen_button = new QPushButton;
		full_screen_button->setObjectName("btn_1");
		full_screen_button->setIcon(QIcon("Images/Slice/full-screen.png"));

		QPushButton* guideLine_button = new QPushButton;
		guideLine_button->setObjectName("btn_2");
		guideLine_button->setIcon(QIcon("Images/Slice/mpr.png"));

		QPushButton* cpr_button = new QPushButton;
		cpr_button->setObjectName("btn_3");
		cpr_button->setIcon(QIcon("Images/Slice/cpr.png"));

		connect(full_screen_button, SIGNAL(clicked()), this, SIGNAL(sigReqFullScreen()));
		connect(guideLine_button, SIGNAL(clicked()), this, SLOT(slotChangeGuideLine()));
		connect(cpr_button, SIGNAL(clicked()), this, SIGNAL(sigReqCPRScreen()));


		m_others_box->addWidget(full_screen_button);
		m_others_box->addWidget(guideLine_button);
		m_others_box->addWidget(cpr_button);

		m_bottom_box->addLayout(m_others_box, Qt::AlignRight);

		m_button_layout->addLayout(m_bottom_box, Qt::AlignBottom);

		mprGuideLine = new MPR_GuideLine(slice_type);

		float fTranslate = (m_img->width() < m_img->height() ? m_img->width() : m_img->height()) * 0.45f;
		float marginCenter = (this->mapToScene(QPoint(0, fTranslate * 0.1)) - this->mapToScene(QPoint(0, 0))).y();
		float marginTranslate = (this->mapToScene(QPoint(0, fTranslate)) - this->mapToScene(QPoint(0, 0))).y();
		float marginRotate = (this->mapToScene(QPoint(0, fTranslate + 20)) - this->mapToScene(QPoint(0, 0))).y();

		mprGuideLine->setLine(this->mapToScene(m_center_origin), marginCenter, marginTranslate, marginRotate);
		mprGuideLine->setCenter(m_center_origin);
		mprGuideLine->addLineToScene(this->scene());
		mprGuideLine->setLineVisible(true);
	}

	//m_windowing_box = _setInfoBox(0, false);

	if (m_slice_type == AXIAL_SLICE) {
		//m_slice_num_box = _setInfoBox(0);
		m_fps_box = _setFPSBox(0);
		m_coord_box = _setInfoBox(1);
		m_func_info_box = _setInfoBox(2);
		m_zoom_ratio_box = _setZoomBox(3);
		input_zoom_ratio = _setEditBox(3);
		//_setEditBox(3);
	}
	else {
		m_fps_box = _setFPSBox(0);
		m_coord_box = _setInfoBox(1);
		m_func_info_box = _setInfoBox(2);
		m_zoom_ratio_box = _setZoomBox(3);
		input_zoom_ratio = _setEditBox(3);
		//_setEditBox(3);
	}
	m_coord_box->setVisible(false);
	input_zoom_ratio->setVisible(false);
	m_zoom_ratio_box->setVisible(true);
	m_fps_box->setVisible(true);

	
	
	m_mode = -1;
	is_draw_guideline = true;
	is_mouse_pressed = false;
	is_horizontal_pressed = false;
	is_vertical_pressed = false;
	is_horizontal_mouseover = false;
	is_vertical_mouseover = false;

	connect(m_zoom_ratio_box, SIGNAL(clicked()), this, SLOT(slotZoomRatioClicked()));
	connect(input_zoom_ratio, SIGNAL(editingFinished()), this, SLOT(slotChangeZoomRatio()));
	//connect(input_zoom_ratio, SIGNAL(textEdited(const QString& text)), this, SLOT(slotChangeZoomRatio(const QString& text)));
}

void SliceWidget::setMode(int m) {
	m_mode = m;

	if (m_mode == MPR_CURVE) {
		is_draw_guideline = false;
		m_cpr_on = true;
		emit sigCPRInit();
	}
	else {
		is_draw_guideline = true;
		m_cpr_on = false;
	}

	is_draw_guideline = true;
	m_cpr_on = false;


	std::cout << "current mode: " << m_mode << std::endl;

}



void SliceWidget::toggleDrawGuideLine() {
	is_draw_guideline = (is_draw_guideline) ? false : true;

	if (!is_draw_guideline) {
		is_horizontal_pressed = false;
		is_vertical_pressed = false;
		is_horizontal_mouseover = false;
		is_vertical_mouseover = false;
	}
}

void SliceWidget::setSlice(SlicePacket slice, cgip::CgipPlane plane) {
	m_slice_packet = slice;
	m_mpr_plane = plane;

	_setScreen();
	_setWindowing();
	//_setPadding();
	//_setGuideLine();
	*m_img = m_windowed_img->copy();
	m_rendered_img->setPixmap(*m_img);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
	fTranslate = (m_img->width() < m_img->height() ? m_img->width() : m_img->height()) * 0.45f;
	marginCenter = (this->mapToScene(QPoint(0, fTranslate * 0.1)) - this->mapToScene(QPoint(0, 0))).y();
	marginTranslate = (this->mapToScene(QPoint(0, fTranslate)) - this->mapToScene(QPoint(0, 0))).y();
	marginRotate = (this->mapToScene(QPoint(0, fTranslate + 20)) - this->mapToScene(QPoint(0, 0))).y();
	//mprGuideLine->setLine(this->mapToScene(m_center_origin), marginCenter, marginTranslate, marginRotate);
	_setNewGuideLine();

	//_setCPRCurve();
	

}

void SliceWidget::setGuideLine(SlicePacket slice, cgip::CgipPlane plane) {
	m_slice_packet = slice;
	m_mpr_plane = plane;

	//_setGuideLine();
	_setNewGuideLine();
}

void SliceWidget::setGuideLine(SlicePacket slice) {
	m_slice_packet = slice;

	//_setGuideLine();
	_setNewGuideLine();
}

/* ========================================================================== */
/*									QT Slots								  */
/* ========================================================================== */

void SliceWidget::slotChangeGuideLine() {
	toggleDrawGuideLine();
	_setSlice();
}

void SliceWidget::slotZoomRatioClicked() {
	m_zoom_ratio_box->setVisible(false);
	input_zoom_ratio->setVisible(true);
}

void SliceWidget::slotChangeZoomRatio() {

	if (input_zoom_ratio->text().length() > 0 ) {
		QString zoomRatio;
		zoomRatio = "x" + input_zoom_ratio->text();
		m_zoom_ratio_box->setText(zoomRatio);
		printf("input_ratio: %f", input_zoom_ratio->text().toDouble());
		emit(sigZoomRatioChanged(input_zoom_ratio->text().toDouble()));
		
	}

	m_zoom_ratio_box->setVisible(true);
	input_zoom_ratio->setVisible(false);
}

/* ========================================================================== */
/* 							   Private Functions							  */
/* ========================================================================== */
void SliceWidget::_setSlice() {
	_setScreen();
	_setWindowing();
	///_setPadding();
	//_setGuideLine();
	*m_img = m_windowed_img->copy();
	m_rendered_img->setPixmap(*m_img);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
	fTranslate = (m_img->width() < m_img->height() ? m_img->width() : m_img->height()) * 0.45f;
	marginCenter = (this->mapToScene(QPoint(0, fTranslate * 0.1)) - this->mapToScene(QPoint(0, 0))).y();
	marginTranslate = (this->mapToScene(QPoint(0, fTranslate)) - this->mapToScene(QPoint(0, 0))).y();
	marginRotate = (this->mapToScene(QPoint(0, fTranslate + 20)) - this->mapToScene(QPoint(0, 0))).y();
	_setNewGuideLine();
	//_setCPRCurve();
	
	

}

void SliceWidget::_setCPRCurve() {
	if (m_mode == MPR_CURVE && cpr_curve != nullptr) {
		std::cout << "coming through for some reason" << std::endl;
		*m_img = m_windowed_img->copy();

		QPainter* boxPainter = new QPainter(m_img);
		pen_forNormalBox.setCosmetic(true);
		boxPainter->setPen(pen_forNormalBox);
		boxPainter->setBrush(brush_forNormalBox);

		cpr_curve->drawPoints(boxPainter);
		cpr_curve->drawCurve(boxPainter);

		boxPainter->setPen(pen_forPath);
		cpr_curve->drawPath(boxPainter);

		boxPainter->setPen(pen_forIndicatorCircle);
		boxPainter->setBrush(brush_forIndicatorCircle);

		cpr_curve->drawIndicatorCircle(boxPainter);



		boxPainter->end();
		SAFE_DELETE_OBJECT(boxPainter);
	}
}

void SliceWidget::_setScreen() {
	m_slice_width = m_mpr_plane.getWidth();
	m_slice_height = m_mpr_plane.getHeight();

	m_cropped_width = m_slice_width; //*this->width() / fmax(this->width(), this->height());
	m_cropped_height = m_slice_height; //*this->height() / fmax(this->width(), this->height());
}

void SliceWidget::_setWindowing() {
	int num_channel = m_mpr_plane.getChannel();
	QImage img(m_cropped_width, m_cropped_height, QImage::Format_RGB32);

#pragma omp parallel for
	for (int xy = 0; xy < m_cropped_width * m_cropped_height; xy++) {
		int y = xy / m_cropped_width;
		int x = xy % m_cropped_width;
		QPointF img_coord = _window_to_slice(QPointF(x, y));

		int temp_r, temp_g, temp_b;
		if (num_channel == 1) {
			temp_r = m_mpr_plane.getPixelValue(img_coord.x(), img_coord.y());
			temp_g = temp_r;
			temp_b = temp_r;
		}
		else if (num_channel == 3) {
			temp_r = m_mpr_plane.getPixelValue(img_coord.x(), img_coord.y(), 0);
			temp_g = m_mpr_plane.getPixelValue(img_coord.x(), img_coord.y(), 1);
			temp_b = m_mpr_plane.getPixelValue(img_coord.x(), img_coord.y(), 2);
		}
		else {
			std::cout << "Channel is not 1 or 3" << std::endl;
			temp_r = temp_g = temp_b = 0;
		}

		uchar val_r = temp_r;
		uchar val_g = temp_g;
		uchar val_b = temp_b;

		/*
		int mask_idx = m_mpr_mask.getPixelValue(img_coord.x(), img_coord.y());
		if (mask_idx > 0) {
			QString mask_val = QString::fromStdString(m_mask_color_table[mask_idx]);
			val_r = (1 - (float)mask_opacity[mask_idx] / 10) * val_r + ((float)mask_opacity[mask_idx] / 10) * QColor(mask_val).red();
			val_g = (1 - (float)mask_opacity[mask_idx] / 10) * val_g + ((float)mask_opacity[mask_idx] / 10) * QColor(mask_val).green();
			val_b = (1 - (float)mask_opacity[mask_idx] / 10) * val_b + ((float)mask_opacity[mask_idx] / 10) * QColor(mask_val).blue();
		}
		*/

		img.setPixel(x, y, qRgb(val_r, val_g, val_b));
	}

	//m_windowed_img = new QPixmap(m_cropped_width, m_cropped_height);
	*m_windowed_img = QPixmap::fromImage(img);
	if (this->height() > this->width()) {
		*m_windowed_img = m_windowed_img->scaledToWidth(this->width(),  Qt::SmoothTransformation);
		m_height_scaled = m_windowed_img->height();
	}
	else {
		*m_windowed_img = m_windowed_img->scaledToHeight(this->height(), Qt::SmoothTransformation);
		m_width_scaled = m_windowed_img->width();
	}
}

void SliceWidget::_setPadding() {
	int padWidth = this->width() - m_windowed_img->width();
	QImage paddedImage(m_windowed_img->width() + padWidth, m_windowed_img->height(), m_windowed_img->toImage().format());
	paddedImage.fill(Qt::black);
	QPainter p(&paddedImage);
	p.drawImage(QPoint(padWidth/2, 0), m_windowed_img->toImage());
	*m_windowed_img = QPixmap::fromImage(paddedImage);

}

/*
int SliceWidget::_applyWindowing(int v) {
	v -= m_window_level - m_window_width / 2;

	if (v < 0)
		v = 0;
	else if (v > m_window_width)
		v = m_window_width;

	v *= 255;
	v /= m_window_width;

	return v;
}
*/

void SliceWidget::_setGuideLine(float radius, float thickness) {
	*m_img = m_windowed_img->copy();

	QPainter* painter = new QPainter(m_img);
	//int padWidth = this->width() - m_windowed_img->width();
	//m_slice_width += padWidth;
	//m_cropped_width += padWidth;

	if (is_draw_guideline) {
		float c_x, c_y;
		std::tie(c_x, c_y, m_horizontal_angle_rad) = m_slice_packet.slice_line;
		float h_width, v_width;
		std::tie(h_width, v_width) = m_slice_packet.thickness;
		h_width = m_slice_width * h_width;
		v_width = m_slice_width * v_width;

		float horizontal_angle_deg = _deg(m_horizontal_angle_rad);
		m_center = _slice_to_window(QPointF(c_x * m_slice_width, c_y * m_slice_height));
		m_center = QPointF(c_x * m_slice_width, c_y * m_slice_height);
		m_center = QPointF(m_center.x() * this->height() / m_cropped_width,
						   m_center.y() * this->height() / m_cropped_width);
		
		float margin = radius + thickness + 5;

		QLineF lines[4];
		lines[0].setP1(QPointF(m_center.x(), m_center.y()));
		lines[0].setAngle(horizontal_angle_deg);
		lines[0].translate(QPointF(margin * cos(m_horizontal_angle_rad), -margin * sin(m_horizontal_angle_rad)));
		lines[0].setLength(this->width() * 5);

		lines[1].setP1(QPointF(m_center.x(), m_center.y()));
		lines[1].setAngle(horizontal_angle_deg + 90);
		lines[1].translate(QPointF(margin * cos(m_horizontal_angle_rad + PI / 2), -margin * sin(m_horizontal_angle_rad + PI / 2)));
		lines[1].setLength(this->width() * 5);

		lines[2].setP1(QPointF(m_center.x(), m_center.y()));
		lines[2].setAngle(horizontal_angle_deg + 180);
		lines[2].translate(QPointF(margin * cos(m_horizontal_angle_rad + PI), -margin * sin(m_horizontal_angle_rad + PI)));
		lines[2].setLength(this->width() * 5);

		lines[3].setP1(QPointF(m_center.x(), m_center.y()));
		lines[3].setAngle(horizontal_angle_deg + 270);
		lines[3].translate(QPointF(margin * cos(m_horizontal_angle_rad + 3 * PI / 2), -margin * sin(m_horizontal_angle_rad + 3 * PI / 2)));
		lines[3].setLength(this->width() * 5);

		// horizontal thickness line
		QColor slab_color;
		QRect slab_rect;

		if (m_slice_type == AXIAL_SLICE)
			slab_color = QColor(0, 0, 0xFF, 0x60);
		else if (m_slice_type == CORONAL_SLICE)
			slab_color = QColor(0xFF, 0, 0, 0x60);
		else if (m_slice_type == SAGITTAL_SLICE)
			slab_color = QColor(0xFF, 0, 0, 0x60);

		slab_rect = QRect(lines[0].p1().x(), lines[0].p1().y() - h_width /2, lines[2].length(), h_width);
		painter->fillRect(slab_rect, slab_color);
		slab_rect.moveTopRight(QPoint(lines[2].p1().x(), lines[2].p1().y() - h_width /2));
		painter->fillRect(slab_rect, slab_color);

		// vertical thickness line
		if (m_slice_type == AXIAL_SLICE)
			slab_color = QColor(0, 0xFF, 0, 0x60);
		else if (m_slice_type == CORONAL_SLICE)
			slab_color = QColor(0, 0xFF, 0, 0x60);
		else if (m_slice_type == SAGITTAL_SLICE)
			slab_color = QColor(0, 0, 0xFF, 0x60);

		slab_rect = QRect(0, 0, v_width, lines[2].length());
		slab_rect.moveBottomRight(QPoint(lines[1].p1().x() + v_width / 2, lines[1].p1().y()));
		painter->fillRect(slab_rect, slab_color);
		slab_rect.moveTopRight(QPoint(lines[3].p1().x() + v_width / 2, lines[3].p1().y()));
		painter->fillRect(slab_rect, slab_color);

		// center
		if ((is_horizontal_pressed && is_vertical_pressed) ||
			(!is_horizontal_pressed && !is_vertical_pressed) && (is_horizontal_mouseover && is_vertical_mouseover)) {
			painter->setPen(QPen(Qt::yellow, thickness));
			painter->drawPoint(m_center);

			painter->setPen(QPen(Qt::yellow, thickness));
			painter->drawEllipse(m_center, radius, radius);
		}

		// horizontal line
		float thickness_h = thickness;
		if (is_horizontal_pressed || (!is_vertical_pressed && is_horizontal_mouseover))
			thickness_h += 5;

		if (m_slice_type == AXIAL_SLICE)
			painter->setPen(QPen(Qt::blue, thickness_h));
		else if (m_slice_type == CORONAL_SLICE)
			painter->setPen(QPen(Qt::red, thickness_h));
		else if (m_slice_type == SAGITTAL_SLICE)
			painter->setPen(QPen(Qt::red, thickness_h));

		painter->drawLine(lines[0]);
		painter->drawLine(lines[2]);

		// vertical line
		float thickness_v = thickness;
		if (is_vertical_pressed || (!is_horizontal_pressed && is_vertical_mouseover))
			thickness_v += 5;

		if (m_slice_type == AXIAL_SLICE)
			painter->setPen(QPen(Qt::green, thickness_v));
		else if (m_slice_type == CORONAL_SLICE)
			painter->setPen(QPen(Qt::green, thickness_v));
		else if (m_slice_type == SAGITTAL_SLICE)
			painter->setPen(QPen(Qt::blue, thickness_v));

		painter->drawLine(lines[1]);
		painter->drawLine(lines[3]);
	}

	painter->end();
	SAFE_DELETE_OBJECT(painter);
}
void SliceWidget::_setNewGuideLine(float radius, float thickness) {
	if (is_draw_guideline) {
		float c_x, c_y;
		std::tie(c_x, c_y, m_horizontal_angle_rad) = m_slice_packet.slice_line;
		float h_width, v_width;
		std::tie(h_width, v_width) = m_slice_packet.thickness;
		h_width = m_slice_width * h_width;
		v_width = m_slice_width * v_width;

		float horizontal_angle_deg = _deg(m_horizontal_angle_rad);
		if (m_slice_width == m_slice_height) {
			m_center = QPointF(c_x * m_slice_width, c_y * m_slice_height);
			m_center = QPointF(m_center.x() * this->height() / m_cropped_width,
							   m_center.y() * this->height() / m_cropped_width);
		}
		else {
			m_center = QPointF(c_x * this->width(), c_y * this->height());
			m_center = QPointF(m_center.x(), m_center.y());
		}

		float margin = radius + thickness + 5;
		mprGuideLine->setLineVisible(true);
		mprGuideLine->setCenterEllipseVisible(true);
		mprGuideLine->setCenter(m_center);
		mprGuideLine->rotate(-horizontal_angle_deg);
		mprGuideLine->setHorizontalThickness(h_width);
		mprGuideLine->setVerticalThickness(v_width);

		//horizontal mouse over
		if (is_horizontal_pressed || (!is_vertical_pressed && is_horizontal_mouseover)) {
			mprGuideLine->setHorizontalLineWidth(5.0f);
			mprGuideLine->setLine(this->mapToScene(m_center_origin), marginCenter, marginTranslate, marginRotate);

		}
		else {
			mprGuideLine->setHorizontalLineWidth(3.0f);
			mprGuideLine->setLine(this->mapToScene(m_center_origin), marginCenter, marginTranslate, marginRotate);

		}
		//vertical mouse over
		if (is_vertical_pressed || (!is_horizontal_pressed && is_vertical_mouseover)) {
			mprGuideLine->setVerticalLineWidth(5.0f);
			mprGuideLine->setLine(this->mapToScene(m_center_origin), marginCenter, marginTranslate, marginRotate);

		}
		else {
			mprGuideLine->setVerticalLineWidth(3.0f);
			mprGuideLine->setLine(this->mapToScene(m_center_origin), marginCenter, marginTranslate, marginRotate);

		}

		//center ellipse mouse over
		if ((is_horizontal_pressed && is_vertical_pressed) ||
			(!is_horizontal_pressed && !is_vertical_pressed) && (is_horizontal_mouseover && is_vertical_mouseover)) {
			mprGuideLine->setCenterEllipseVisible(true);
		}
		else {
			mprGuideLine->setCenterEllipseVisible(false);
		}




	}
	else {
		mprGuideLine->setLineVisible(false);
		mprGuideLine->setCenterEllipseVisible(false);
	}
}


QLabel* SliceWidget::_setInfoBox(int i, bool align_top) {
	// Coord Info
	QLabel* info_box = new QLabel(this);
	info_box->setMouseTracking(true);
	info_box->setFixedWidth(300);

	if (align_top) {
		info_box->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		info_box->move(10, 20 * (i+1));
	}
	else {
		info_box->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
		info_box->move(10, this->height() - (20*(i+1)));
	}

	//info_box->setMargin(8);

	QPalette pal = palette();
	pal.setColor(info_box->foregroundRole(), Qt::green);
	info_box->setPalette(pal);

	return info_box;
}

ClickableLabel* SliceWidget::_setZoomBox(int i, bool align_top) {
	// Coord Info
	ClickableLabel* info_box = new ClickableLabel(this);
	info_box->setMouseTracking(true);
	info_box->setFixedWidth(70);

	if (align_top) {
		info_box->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		info_box->move(10, 20 * (i + 1));
	}
	else {
		info_box->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
		info_box->move(10, this->height() - (20 * (i + 1)));
	}

	//info_box->setMargin(8);

	QPalette pal = palette();
	pal.setColor(info_box->foregroundRole(), Qt::green);
	info_box->setPalette(pal);

	return info_box;
}

QLineEdit* SliceWidget::_setEditBox(int i, bool align_top) {
	// Coord Info
	QLineEdit* info_box= new QLineEdit(this);
	info_box->setMouseTracking(true);
	info_box->setFixedWidth(70);

	if (align_top) {
		info_box->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		info_box->move(10, 20 * (i + 1));
	}
	else {
		info_box->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
		info_box->move(10, this->height() - (20 * (i + 1)));
	}

	return info_box;
}



void SliceWidget::setCoordInfo(float x, float y) {
	QPointF point = _window_to_slice(QPointF(x * m_cropped_width / this->width(),
											y * m_cropped_width / this->width()));

	//int val = m_mpr_plane.getPixelValue(point.x(), point.y());
	/*
	float x_norm, y_norm;
	if (this->width() >= this->height()) {
		x_norm = x / this->width();
		y_norm = (y + (this->width() - this->height()) / 2) / this->width();
	}
	else {
		x_norm = (x + (this->height() - this->width()) / 2) / this->height();
		y_norm = y / this->height();
	}
	*/

	QString msg;
	msg = "2D: (" + QString::number((int)point.x()) + ", " + QString::number((int)point.y()) + ")";
	m_coord_box->setText(msg);

	setSliceInfo(point.x(), point.y());
}

float SliceWidget::round_float(float var) {
	float value = (int)(var * 100 + .5);
	return (float)value / 100;
}


void SliceWidget::setZoomRatioInfo(float ratio) {

	QString zoomRatio;
	zoomRatio = "x" + QString::number(round_float(ratio));
	m_zoom_ratio_box->setText(zoomRatio);
	input_zoom_ratio->setText(QString::number(round_float(ratio)));

}


QLabel* SliceWidget::_setFPSBox(int i, bool align_top) {
	QLabel* info_box = new QLabel(this);
	info_box->setFixedWidth(200);

	if (align_top) {
		info_box->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		info_box->move(10, 20 * (i + 1));
	}
	else {
		info_box->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
		info_box->move(10, this->height() - (20 * (i + 1)));
	}

	//info_box->setMargin(8);

	QPalette pal = palette();
	pal.setColor(info_box->foregroundRole(), Qt::green);
	info_box->setPalette(pal);

	QString msg;
	msg = "FPS: ";
	info_box->setText(msg);

	return info_box;
}

void SliceWidget::setFPSInfo(int fps) {
	QString msg;
	msg = "FPS: " + QString::number(fps);
	m_fps_box->setText(msg);
	
}


void SliceWidget::setSliceInfo(float x, float y) {
	if (m_slice_type == AXIAL_SLICE) {
		//cgip::CgipPoint point_3d = m_mpr_mgr->getVolumeCoordInfo(m_slice_type, x, y);
		//QString msg = "Slice: " + QString::number(round(point_3d.z()) + 1);
		//m_slice_num_box->setText(msg);
	}
}

void SliceWidget::_setWindowingInfo() {
	QString msg;
	msg = "windowing center: " + QString::number(m_window_level) + ", width: " + QString::number(m_window_width);
	//m_windowing_box->setText(msg);
}

void SliceWidget::resizeEvent(QResizeEvent* event) {
	if (m_mode < 0)
		m_rendered_img->setPixmap(m_img->scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	else {
		_setSlice();

	}
	//m_windowing_box->move(10, this->height()-40);
}

ClickType SliceWidget::_get_clicktype(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
		return LEFT_CLICK;
	if (e->button() == Qt::RightButton)
		return RIGHT_CLICK;
	if (e->button() == Qt::MidButton)
		return MIDDLE_CLICK;
}

float SliceWidget::_get_mouse_angle(QPointF p) {
	float mouse_angle_rad;
	if (fabs(p.x() - m_center.x()) < EPSILON) {
		if (p.y() >= m_center.y())
			mouse_angle_rad = PI / 2;
		else
			mouse_angle_rad = 3 * PI / 2;
	}
	else if (fabs(p.y() - m_center.y()) < EPSILON) {
		if (p.x() >= m_center.x())
			mouse_angle_rad = 0;
		else
			mouse_angle_rad = PI;
	}
	else if (p.x() > m_center.x() && p.y() > m_center.y())
		mouse_angle_rad = atan2(p.y() - m_center.y(), p.x() - m_center.x());
	else if (p.x() < m_center.x() && p.y() > m_center.y())
		mouse_angle_rad = atan2(m_center.x() - p.x(), p.y() - m_center.y()) + PI / 2;
	else if (p.x() < m_center.x() && p.y() < m_center.y())
		mouse_angle_rad = atan2(m_center.y() - p.y(), m_center.x() - p.x()) + PI;
	else
		mouse_angle_rad = atan2(p.x() - m_center.x(), m_center.y() - p.y()) + 3 * PI / 2;

	return mouse_angle_rad;
}

void SliceWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_mode < 0)
		return;

	QPointF cur_pos = event->pos();
	
	// TODO: emit coord info of cursor location
	//emit sigCoordInfo(cur_pose.x(), cur_pos.y(), this);

	setCoordInfo(cur_pos.x(), cur_pos.y());
	m_coord_box->setVisible(true);

	if (m_mode > 1) {
		QPointF point = _window_to_slice(QPointF(cur_pos.x() * m_cropped_width / this->width(),
												 cur_pos.y() * m_cropped_width / this->width()));
		return;
	}

	// check if cursor is on guideline
	if (is_draw_guideline && m_mode==0 && !is_mouse_pressed) {
		float horizontal_dist = (abs(tan(PI - m_horizontal_angle_rad) * (this->mapToScene(event->pos()).x() - m_center.x()) + m_center.y() - this->mapToScene(event->pos()).y()) / sqrt(pow(tan(PI - m_horizontal_angle_rad), 2) + 1));
		float vertical_dist = (abs(tan(3 * PI / 2 - m_horizontal_angle_rad) * (this->mapToScene(event->pos()).x() - m_center.x()) + m_center.y() - this->mapToScene(event->pos()).y()) / sqrt(pow(tan(3 * PI / 2 - m_horizontal_angle_rad), 2) + 1));
		bool cur_horizontal_mouseover = horizontal_dist < DELTA;
		bool cur_vertical_mouseover = vertical_dist < DELTA;

		// update guideline mouseover states
		if (is_horizontal_mouseover != cur_horizontal_mouseover || is_vertical_mouseover != cur_vertical_mouseover) {
			is_horizontal_mouseover = cur_horizontal_mouseover;
			is_vertical_mouseover = cur_vertical_mouseover;

			if (is_horizontal_mouseover || is_vertical_mouseover) {
				//std::cout << "mouse hovering on guideline..." << std::endl;
			}

			setGuideLine(m_slice_packet, m_mpr_plane);
		}
	}

	if (!is_mouse_pressed)
		return;

	// TODO: when mouse leaves widget during clicked?
	float max_size = fmax(this->width(), this->height());
	float dx = cur_pos.x() - m_pressed_loc.x();
	float dy = cur_pos.y() - m_pressed_loc.y();
	float dx_norm = dx / max_size;
	float dy_norm = dy / max_size;
	if (m_pressed_type == LEFT_CLICK) {
		if (is_horizontal_pressed || is_vertical_pressed) { // move guideline
			float x, y;
			if (m_slice_width == m_slice_height) {
				if (this->width() >= this->height()) {
					//std::cout << "guideline moving.... " << std::endl;
					x = cur_pos.x() / this->width();
					y = (cur_pos.y() + (this->width() - this->height()) / 2) / this->width();
				}
				else {
					x = cur_pos.x() / this->height();
					y = (cur_pos.y() + (this->height() - this->width()) / 2) / this->height();
				}
			}
			else {
				x = cur_pos.x() / this->width();
				y = cur_pos.y() / this->height();
			}

			if (Qt::ControlModifier & event->modifiers()) {
				// thickness
				if (is_horizontal_pressed) {
					emit sigGuideLineThickness(dy, true, false);
				}

				if (is_vertical_pressed) {
					emit sigGuideLineThickness(dx, false, true);
				}

			}
			else {
				//std::cout << "guideline move x: " << x << ", " << y << std::endl;
				emit sigGuideLineMoved(is_horizontal_pressed, is_vertical_pressed, x, y);
			}
		}
		else { // zooming
			//emit sigScreenDragged(dx_norm, dy_norm, m_pressed_type);
		}
	}
	else if (m_pressed_type == RIGHT_CLICK) {
		if (!(is_horizontal_pressed || is_vertical_pressed)) { // windowing
			//emit sigScreenDragged(dx, dy, m_pressed_type);
			//_setWindowingInfo();
		}
		else if (!(is_horizontal_pressed && is_vertical_pressed)) { // rotate guideline
			int rot_direction = 1;
			float cur_mouse_angle_rad = _get_mouse_angle(cur_pos);
			float angle_diff = fabs(cur_mouse_angle_rad - m_pressed_angle_rad);
			if (angle_diff > PI)
				angle_diff = 2 * PI - angle_diff;

			float angle_temp = cur_mouse_angle_rad - m_pressed_angle_rad;
			if (angle_temp < 0)
				angle_temp += 2 * PI;
			if (angle_temp < PI)
				rot_direction = -1;

			emit sigGuideLineRotated(rot_direction * angle_diff);

			m_pressed_angle_rad = cur_mouse_angle_rad;
		}
	}
	else if (m_pressed_type == MIDDLE_CLICK) { // panning
		emit sigScreenDragged(-dx_norm, -dy_norm, m_pressed_type);
	}

	m_pressed_loc = cur_pos;
}

void SliceWidget::mousePressEvent(QMouseEvent* event) {
	if (m_mode < 0)
		return;

	if (is_mouse_pressed)
		return;

	is_mouse_pressed = true;
	m_pressed_type = _get_clicktype(event);
	m_pressed_loc = event->localPos();
	m_pressed_angle_rad = _get_mouse_angle(m_pressed_loc);

	// set guidelines pressed states
	if (is_horizontal_mouseover)
		is_horizontal_pressed = true;
	if (is_vertical_mouseover)
		is_vertical_pressed = true;

	if (is_horizontal_pressed || is_vertical_pressed) {
		emit sigClicked(0, 0, LEFT_CLICK);
	}
	else if (m_pressed_type == MIDDLE_CLICK) {
		emit sigClicked(0, 0, LEFT_CLICK);
	}

	if (m_mode == CPR) {

	}
}

void SliceWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (m_mode < 0)
		return;

	if (!is_mouse_pressed)
		return;

	QPointF cur_pos = event->localPos();
	//emit sigCoordInfo(cur_pose.x(), cur_pos.y(), this);

	setCoordInfo(cur_pos.x(), cur_pos.y());
	m_coord_box->setVisible(true);

	if (m_mode > 1) {
		QPointF point = _window_to_slice(QPointF(cur_pos.x() * m_cropped_width / this->width(),
			cur_pos.y() * m_cropped_width / this->width()));
		return;
	}

	if (!is_mouse_pressed)
		return;

	ClickType released_type = _get_clicktype(event);
	if (released_type != m_pressed_type)
		return;

	if (is_horizontal_pressed || is_vertical_pressed) {
		emit sigMouseReleased(event->x(), event->y(), LEFT_CLICK);
	}
	else if(m_pressed_type == MIDDLE_CLICK){
		emit sigMouseReleased(event->x(), event->y(), LEFT_CLICK);
	}
	else {
		emit sigMouseReleased(event->x(), event->y(), RIGHT_CLICK);
	}

	is_mouse_pressed = false;
	is_horizontal_pressed = false;
	is_vertical_pressed = false;
}

void SliceWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	if (m_mode < 0)
		return;


	float x, y;
	QPointF cur_pos = event->localPos();
	*m_img = m_windowed_img->copy();

	
	if (this->width() >= this->height()) {
		x = cur_pos.x() / this->width();
		y = (cur_pos.y() + (this->width() - this->height()) / 2) / this->width();
	}
	else {
		x = (cur_pos.x() + (this->height() - this->width()) / 2) / this->height();
		y = cur_pos.y() / this->height();
	}

	emit sigDoubleClicked(x, y, _get_clicktype(event));
	setCoordInfo(event->localPos().x(), event->localPos().y());
	
	
}

void SliceWidget::wheelEvent(QWheelEvent* event) {
	if (m_mode < 0)
		return;

	if (is_mouse_pressed)
		return;

	if (m_mode < 2) {
		emit sigWheelChanged(event->angleDelta().y() > 0 ? -1 : 1);
		setCoordInfo(event->posF().x(), event->posF().y());
	}
}

void SliceWidget::leaveEvent(QEvent *event) {
	// this event does not occur when mouse pressed
	if (m_mode < 0)
		return;

	// if mouse leaves widget, make guilelines thin
	if (is_draw_guideline) {
		is_horizontal_mouseover = false;
		is_vertical_mouseover = false;
		is_horizontal_pressed = false;
		is_vertical_pressed = false;
		//setGuideLine(m_slice_packet, m_mpr_plane);
	}

	m_coord_box->setVisible(false);
}
