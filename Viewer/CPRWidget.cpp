#include "CPRWidget.h"


CPRWidget::CPRWidget(QWidget* parent, const char* name, SliceType slice_type) : QGraphicsView(parent) {
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
	m_pan_center_point = QPointF(m_img->width() / 2.0, m_img->height() / 2.0);
	m_rendered_img->setPos(0, 0);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
	this->scale(1.0, 1.0);

	m_windowed_img = new QPixmap();


	
	
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

	//connect(plus_button, &QPushButton::clicked, [this] {emit sigCurveZoomed(0, -0.05, LEFT_CLICK); });
	//connect(minus_button, &QPushButton::clicked, [this] {emit sigCurveZoomed(0, 0.05, LEFT_CLICK); });
	connect(plus_button, &QPushButton::clicked, this, &CPRWidget::slotZoomIn);
	connect(minus_button, &QPushButton::clicked, this, &CPRWidget::slotZoomOut);



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
		if (m_wl_slider->isSliderDown())
			emit sigSliderDragged(m_ww_slider->value(), m_wl_slider->value(), RIGHT_CLICK);
		});
	connect(m_ww_slider, &QSlider::valueChanged, [=](int v) {
		ww_val->setText(QString::number(v));
		if (m_ww_slider->isSliderDown())
			emit sigSliderDragged(m_ww_slider->value(), m_wl_slider->value(), RIGHT_CLICK);
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

	// full screen / guideline / CPR
	QHBoxLayout* m_others_box = new QHBoxLayout;
	m_others_box->setAlignment(Qt::AlignBottom | Qt::AlignRight);

	m_bottom_box->addLayout(m_others_box, Qt::AlignRight);

	m_button_layout->addLayout(m_bottom_box, Qt::AlignBottom);

	
	
	

	//m_windowing_box = _setInfoBox(0, false);

	m_fps_box = _setFPSBox(0);
	m_coord_box = _setInfoBox(1);
	m_func_info_box = _setInfoBox(2);
	m_zoom_ratio_box = _setZoomBox(3);
	input_zoom_ratio = _setEditBox(3);

	
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

void CPRWidget::setMode(int m) {
	m_mode = m;
}
void CPRWidget::toggleDrawGuideLine() {
	is_draw_guideline = (is_draw_guideline) ? false : true;

	if (!is_draw_guideline) {
		is_horizontal_pressed = false;
		is_vertical_pressed = false;
		is_horizontal_mouseover = false;
		is_vertical_mouseover = false;
	}
}

void CPRWidget::setSlice(SlicePacket slice, cgip::CgipPlane plane) {
	m_slice_packet = slice;
	m_mpr_plane = plane;

	if (!loaded_slice) {
		_setScreen();
		loaded_slice = true;
	}
	_setWindowing();
	//_setGuideLine();

	*m_img = m_windowed_img->copy();
	m_rendered_img->setPixmap(*m_img);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_pan_center_point = QPointF(m_img->width() / 2.0, m_img->height() / 2.0);

	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);

}

void CPRWidget::resetSlice() {

	if (m_img) {
		SAFE_DELETE_OBJECT(m_img);
	}

	if (m_windowed_img) {
		SAFE_DELETE_OBJECT(m_windowed_img);
	}

	m_img = new QPixmap(512, 512);
	m_img->fill(QColor("black"));

	m_rendered_img = this->scene()->addPixmap(m_img->scaledToWidth(this->width(), Qt::SmoothTransformation));
	m_rendered_img->setPos(0, 0);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_pan_center_point = QPointF(m_img->width() / 2.0, m_img->height() / 2.0);

	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);

	m_windowed_img = new QPixmap();
}




/* ========================================================================== */
/*									QT Slots								  */
/* ========================================================================== */


void CPRWidget::slotZoomIn() {
	float zoom_in_by = 0.05;
	m_zoomRatio += zoom_in_by;
	float factor = m_zoomRatio;
	QTransform tr = this->transform();
	tr.setMatrix(factor, tr.m12(), tr.m13(), tr.m21(), factor, tr.m23(), tr.m31(), tr.m32(), tr.m33());
	this->setTransform(tr);
}

void CPRWidget::slotZoomOut() {
	float zoom_out_by = 0.05;
	m_zoomRatio -= zoom_out_by;
	float factor = m_zoomRatio;
	QTransform tr = this->transform();
	tr.setMatrix(factor, tr.m12(), tr.m13(), tr.m21(), factor, tr.m23(), tr.m31(), tr.m32(), tr.m33());
	this->setTransform(tr);
}


void CPRWidget::slotZoomRatioClicked() {
	m_zoom_ratio_box->setVisible(false);
	input_zoom_ratio->setVisible(true);
}

void CPRWidget::slotChangeZoomRatio() {

	if (input_zoom_ratio->text().length() > 0) {
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
void CPRWidget::_setSlice() {
	_setScreen();
	_setWindowing();
	*m_img = m_windowed_img->copy();
	m_rendered_img->setPixmap(*m_img);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
}

void CPRWidget::_setScreen() {
	m_slice_width = m_mpr_plane.getWidth();
	m_slice_height = m_mpr_plane.getHeight();

	m_cropped_width = m_slice_width; //* this->width() / fmax(this->width(), this->height());
	m_cropped_height = m_slice_height; //* this->height() / fmax(this->width(), this->height());
}

void CPRWidget::_setWindowing() {

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
	*m_windowed_img = m_windowed_img->scaledToHeight(this->height(), Qt::SmoothTransformation);
}

/*
int CPRWidget::_applyWindowing(int v) {
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


QLabel* CPRWidget::_setInfoBox(int i, bool align_top) {
	// Coord Info
	QLabel* info_box = new QLabel(this);
	info_box->setMouseTracking(true);
	info_box->setFixedWidth(300);

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

ClickableLabel* CPRWidget::_setZoomBox(int i, bool align_top) {
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

QLineEdit* CPRWidget::_setEditBox(int i, bool align_top) {
	// Coord Info
	QLineEdit* info_box = new QLineEdit(this);
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



void CPRWidget::setCoordInfo(float x, float y) {
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

float CPRWidget::round_float(float var) {
	float value = (int)(var * 100 + .5);
	return (float)value / 100;
}


void CPRWidget::setZoomRatioInfo(float ratio) {

	QString zoomRatio;
	zoomRatio = "x" + QString::number(round_float(ratio));
	m_zoom_ratio_box->setText(zoomRatio);
	input_zoom_ratio->setText(QString::number(round_float(ratio)));

}


QLabel* CPRWidget::_setFPSBox(int i, bool align_top) {
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

void CPRWidget::setFPSInfo(int fps) {
	QString msg;
	msg = "FPS: " + QString::number(fps);
	m_fps_box->setText(msg);

}


void CPRWidget::setSliceInfo(float x, float y) {
	if (m_slice_type == AXIAL_SLICE) {
		//cgip::CgipPoint point_3d = m_mpr_mgr->getVolumeCoordInfo(m_slice_type, x, y);
		//QString msg = "Slice: " + QString::number(round(point_3d.z()) + 1);
		//m_slice_num_box->setText(msg);
	}
}

void CPRWidget::_setWindowingInfo() {
	QString msg;
	msg = "windowing center: " + QString::number(m_window_level) + ", width: " + QString::number(m_window_width);
	//m_windowing_box->setText(msg);
}
void CPRWidget::resizeEvent(QResizeEvent* event) {

	if (loaded_slice) {
		_setWindowing();
	}
	//_setCPRCurve();
	*m_img = m_windowed_img->copy();
	m_rendered_img->setPixmap(*m_img);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_pan_center_point = QPointF(m_img->width() / 2.0, m_img->height() / 2.0);

	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
	//m_windowing_box->move(10, this->height()-40);
}

ClickType CPRWidget::_get_clicktype(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		return LEFT_CLICK;
	if (e->button() == Qt::RightButton)
		return RIGHT_CLICK;
	if (e->button() == Qt::MidButton)
		return MIDDLE_CLICK;
}

float CPRWidget::_get_mouse_angle(QPointF p) {
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

void CPRWidget::setPanCenterPoint(const QPointF& newCenterPoint)
{
	m_pan_center_point = newCenterPoint;
	this->centerOn(m_pan_center_point);
}

void CPRWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (is_panning) {
		QPointF current_point = this->mapToScene(event->pos());
		QPointF delta_position = current_point - this->mapToScene(last_pan_point);
		last_pan_point = event->pos();

		this->setPanCenterPoint(getPanCenterPoint() - delta_position);

	}
}

void CPRWidget::mousePressEvent(QMouseEvent* event) {
	
	if (event->button() == Qt::MiddleButton) {
		last_pan_point = event->pos();
		is_panning = true;
		std::cout << " wheel click " << std::endl;
	}
}

void CPRWidget::mouseReleaseEvent(QMouseEvent* event) {

	if (is_panning) {
		is_panning = false;
	}
}

void CPRWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	
	if (event->button() == Qt::RightButton) {
		std::cout << "CPR Widget double clicked" << std::endl;

		m_zoomRatio = 1.0;
		float factor = 1.0;
		QTransform tr = this->transform();
		tr.setMatrix(factor, tr.m12(), tr.m13(), tr.m21(), factor, tr.m23(), tr.m31(), tr.m32(), tr.m33());
		this->setTransform(tr);
		this->setPanCenterPoint(QPointF(m_img->width() / 2, m_img->height() / 2));
	}
}

void CPRWidget::wheelEvent(QWheelEvent* event) {
	if (m_mode < 0)
		return;

	if (is_mouse_pressed)
		return;

	if (m_mode < 2) {
		emit sigWheelChanged(event->angleDelta().y() > 0 ? -1 : 1);
		setCoordInfo(event->posF().x(), event->posF().y());
	}
}

void CPRWidget::leaveEvent(QEvent* event) {
	// this event does not occur when mouse pressed
	if (m_mode < 0)
		return;

	
	m_coord_box->setVisible(false);
}
