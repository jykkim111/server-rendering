#include "CPRView.h"
#include <QLabel>
#include <QLayout>
#include <QDebug>
#include <QPainter>
#include <QTimer>

#include <algorithm>
#include <vector>
#include <iostream>


CPRView::CPRView(QWidget* parent, const char* name, SliceType slice_type) : QGraphicsView(parent) {
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
	//this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	this->scene()->setSceneRect(-20000, -20000, 40000, 40000);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_pan_center_point = QPointF(m_img->width() / 2.0, m_img->height() / 2.0);
	m_rendered_img->setPos(0, 0);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
	this->scale(1.0, 1.0);
	float factor = 1.0;
	//m_rendered_img->setPos(this->width() / 2 - m_img->width() / 2, this->height() / 2 - m_img->height() / 2);

	m_windowed_img = new QPixmap();
	
	m_view_width = 0;
	m_view_height = 0;

	m_zoomFactor = 1;
	m_zoomRatio = 1;
	std::cout << " GraphicsView rect: " << this->width() << ", " << this->height() << std::endl;
	std::cout << " GraphicsScene rect: " << this->scene()->sceneRect().width() << ", " << this->scene()->sceneRect().height() << std::endl;


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

	connect(plus_button, &QPushButton::clicked, this, &CPRView::slotZoomIn);
	connect(minus_button, &QPushButton::clicked, this, &CPRView::slotZoomOut);

	m_zoom_box->addWidget(plus_button);
	m_zoom_box->addWidget(minus_button);
	m_button_layout->addLayout(m_zoom_box, Qt::AlignRight | Qt::AlignTop);

	QHBoxLayout* m_bottom_box = new QHBoxLayout;
	m_bottom_box->setAlignment(Qt::AlignBottom);

	// full screen / guideline
	QHBoxLayout* m_others_box = new QHBoxLayout;
	m_others_box->setAlignment(Qt::AlignBottom | Qt::AlignRight);

	QPushButton* full_screen_button = new QPushButton;
	full_screen_button->setObjectName("btn_1");
	full_screen_button->setIcon(QIcon("Images/Slice/full-screen.png"));
	
	connect(full_screen_button, &QPushButton::clicked, this, &CPRView::slotResetTransformation);


	m_others_box->addWidget(full_screen_button);

	m_bottom_box->addLayout(m_others_box, Qt::AlignRight);

	m_button_layout->addLayout(m_bottom_box, Qt::AlignBottom);


	

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
	
	is_mouse_pressed = false;


	//connect(m_zoom_ratio_box, SIGNAL(clicked()), this, SLOT(slotZoomRatioClicked()));
	//connect(input_zoom_ratio, SIGNAL(editingFinished()), this, SLOT(slotChangeZoomRatio()));
	//connect(input_zoom_ratio, SIGNAL(textEdited(const QString& text)), this, SLOT(slotChangeZoomRatio(const QString& text)));
}

void CPRView::setMode(int m, SliceType slice_type) {
	m_mode = m;
	m_slice_type = slice_type;

	if (m_mode == MPR_CURVE) {
		m_cpr_on = true;
		emit sigCPRInit(m_slice_type);
	}
	else {
		m_cpr_on = false;
	}

	std::cout << "current mode: " << m_mode << std::endl;

}

void CPRView::resetSlice() {

	if (m_img) {
		SAFE_DELETE_OBJECT(m_img);
	}

	if (m_windowed_img) {
		SAFE_DELETE_OBJECT(m_windowed_img);
	}

	if (cpr_curve) {
		SAFE_DELETE_OBJECT(cpr_curve);
	}

	if (m_rendered_img) {
		SAFE_DELETE_OBJECT(m_rendered_img);
	}

	m_img = new QPixmap(512, 512);
	m_img->fill(QColor("black"));

	this->setMouseTracking(true);
	this->setBackgroundBrush(Qt::black);

	m_rendered_img = this->scene()->addPixmap(m_img->scaledToWidth(this->width(), Qt::SmoothTransformation));
	m_rendered_img->setPos(0, 0);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_pan_center_point = QPointF(m_img->width() / 2.0, m_img->height() / 2.0);

	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);

	std::cout << "(resetSlice) graphicsview size : " << this->width() << ", " << this->height() << std::endl;
	std::cout << "(resetSlice) m_img_size: " << m_img->width() << ", " << m_img->height() << std::endl;

	//this->fitInView(m_rendered_img, Qt::KeepAspectRatio);
	//m_rendered_img->setPos(-this->width(), this->height() / 2 - m_img->width() / 2);
	
	//m_rendered_img->setPos(this->width() - m_img->width(), this->height()- m_img->height());

	m_windowed_img = new QPixmap();
}

void CPRView::setSlice(SlicePacket slice, cgip::CgipPlane plane) {
	m_slice_packet = slice;
	m_mpr_plane = plane;

	if (!loaded_slice) {
		_setScreen();
		loaded_slice = true;
	}
	_setWindowing();
	//_setCPRCurve();
	*m_img = m_windowed_img->copy();
	m_rendered_img->setPixmap(*m_img);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_pan_center_point = QPointF(m_img->width() / 2.0, m_img->height() / 2.0);

	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
	//this->fitInView(m_rendered_img, Qt::KeepAspectRatio);
	//m_rendered_img->setPos(this->width() / 2 - m_img->width() / 2, this->height() / 2 - m_img->height() / 2);

}



/* ========================================================================== */
/*									QT Slots								  */
/* ========================================================================== */



void CPRView::slotZoomIn() {
	float zoom_in_by = 0.05;
	m_zoomRatio += zoom_in_by;
	float factor = m_zoomRatio;
	QTransform tr = this->transform();
	tr.setMatrix(factor, tr.m12(), tr.m13(), tr.m21(), factor, tr.m23(), tr.m31(), tr.m32(), tr.m33());
	this->setTransform(tr);	


}

void CPRView::slotZoomOut() {
	float zoom_out_by = 0.05;
	m_zoomRatio -= zoom_out_by;
	float factor = m_zoomRatio;
	QTransform tr = this->transform();
	tr.setMatrix(factor, tr.m12(), tr.m13(), tr.m21(), factor, tr.m23(), tr.m31(), tr.m32(), tr.m33());
	this->setTransform(tr);
	
}

void CPRView::slotResetTransformation() {
	m_zoomRatio = 1.0;
	float factor = 1.0;
	QTransform tr = this->transform();
	tr.setMatrix(factor, tr.m12(), tr.m13(), tr.m21(), factor, tr.m23(), tr.m31(), tr.m32(), tr.m33());
	this->setTransform(tr);
	this->setPanCenterPoint(QPointF(m_img->width() / 2, m_img->height() / 2));
}




void CPRView::slotZoomRatioClicked() {
	m_zoom_ratio_box->setVisible(false);
	input_zoom_ratio->setVisible(true);
}

void CPRView::slotChangeZoomRatio() {

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
void CPRView::setPanCenterPoint(const QPointF& newCenterPoint)
{
	m_pan_center_point = newCenterPoint;
	this->centerOn(m_pan_center_point);
}



void CPRView::_setScreen() {
	m_slice_width = m_mpr_plane.getWidth();
	m_slice_height = m_mpr_plane.getHeight();


	//m_cropped_width = m_slice_width * this->width() / fmax(this->width(), this->height());
	//m_cropped_height = m_slice_height * this->height() / fmax(this->width(), this->height());
	m_cropped_width = m_slice_width * this->width() / fmin(this->width(), this->height());
	m_cropped_height = m_slice_height * this->height() / fmin(this->width(), this->height());

	//if (this->width() > this->height()) {
	//	m_cropped_width = m_slice_width * this->width() / fmax(this->width(), this->height());
	//	m_cropped_height = m_slice_height * this->height() / fmax(this->width(), this->height());
	//}
	//else {
	//	m_cropped_width = m_slice_width * this->width() / fmin(this->width(), this->height());
	//	m_cropped_height = m_slice_height * this->height() / fmin(this->width(), this->height());
	//}
	
}

void CPRView::_setWindowing() {
	int num_channel = m_mpr_plane.getChannel();
	QImage img(m_slice_width, m_slice_height, QImage::Format_RGB32);

#pragma omp parallel for
	for (int xy = 0; xy < m_slice_width * m_slice_height; xy++) {
		int y = xy / m_slice_width;
		int x = xy % m_slice_width;
		//QPointF img_coord = _window_to_slice(QPointF(x, y));
		QPointF img_coord = QPointF(x, y);


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
	*m_windowed_img = m_windowed_img->scaledToWidth(this->width(), Qt::SmoothTransformation);
}



QLabel* CPRView::_setInfoBox(int i, bool align_top) {
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

ClickableLabel* CPRView::_setZoomBox(int i, bool align_top) {
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

QLineEdit* CPRView::_setEditBox(int i, bool align_top) {
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



void CPRView::setCoordInfo(float x, float y) {
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

float CPRView::round_float(float var) {
	float value = (int)(var * 100 + .5);
	return (float)value / 100;
}


void CPRView::setZoomRatioInfo(float ratio) {

	QString zoomRatio;
	zoomRatio = "x" + QString::number(round_float(ratio));
	m_zoom_ratio_box->setText(zoomRatio);
	input_zoom_ratio->setText(QString::number(round_float(ratio)));

}


QLabel* CPRView::_setFPSBox(int i, bool align_top) {
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

void CPRView::setFPSInfo(int fps) {
	QString msg;
	msg = "FPS: " + QString::number(fps);
	m_fps_box->setText(msg);

}


void CPRView::setSliceInfo(float x, float y) {
	if (m_slice_type == AXIAL_SLICE) {
		//cgip::CgipPoint point_3d = m_mpr_mgr->getVolumeCoordInfo(m_slice_type, x, y);
		//QString msg = "Slice: " + QString::number(round(point_3d.z()) + 1);
		//m_slice_num_box->setText(msg);
	}
}

void CPRView::_setWindowingInfo() {
	QString msg;
	msg = "windowing center: " + QString::number(m_window_level) + ", width: " + QString::number(m_window_width);
	//m_windowing_box->setText(msg);
}

void CPRView::resizeEvent(QResizeEvent* event) {
	QSize size = event->size();
	
	//this->scene()->setSceneRect(-20000, -20000, 40000, 40000);
	//m_rendered_img->setPos(0, 0);
	

	float ratio = 0.0f;

	//_setSlice();
	_setWindowing();
	//_setCPRCurve();
	*m_img = m_windowed_img->copy();
	m_rendered_img->setPixmap(*m_img);
	float factor = 1.0;	
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_pan_center_point = QPointF(m_img->width() / 2.0, m_img->height() / 2.0);

	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);

	//m_rendered_img->setPos(this->width() - m_img->width(), this->height() - m_img->height());
	std::cout << "graphicsview size : " << this->width() << ", " << this->height() << std::endl;
	std::cout << "m_img_size: " << m_img->width() << ", " << m_img->height() << std::endl;
	std::cout << "set slice to cprview" << std::endl;
}

ClickType CPRView::_get_clicktype(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		return LEFT_CLICK;
	if (e->button() == Qt::RightButton)
		return RIGHT_CLICK;
	if (e->button() == Qt::MidButton)
		return MIDDLE_CLICK;
}

float CPRView::_get_mouse_angle(QPointF p) {
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

void CPRView::slotUpdateCPRPath() {
	emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);
}

void CPRView::mouseMoveEvent(QMouseEvent* event)
{
	if (m_mode < 0)
		return;
	if (is_panning) {
		float pntDiff_x = m_middle_previous_loc.x() - event->pos().x();
		float pntDiff_y = m_middle_previous_loc.y() - event->pos().y();
		//QPointF delta_position = m_pan_center_point - event->pos();
		QPointF current_point = this->mapToScene(event->pos());
		QPointF delta_position = current_point - this->mapToScene(last_pan_point);
		last_pan_point = event->pos();
		
		//std::cout << "pntdiff: " << pntDiff_x << ", " << pntDiff_y << std::endl;

		//this->translate(pntDiff_x, pntDiff_y);

		//int newX = this->horizontalScrollBar()->value() + pntDiff_x;
		//int newY = this->verticalScrollBar()->value() + pntDiff_y;
		//this->horizontalScrollBar()->setValue(newX);
		//this->verticalScrollBar()->setValue(newY);
		//QPointF final_pan_point = this->getPanCenterPoint() - mousePos;
		this->setPanCenterPoint(getPanCenterPoint() - delta_position);

	}
	


	if (m_mode == MPR_CURVE) {

		if (cpr_curve) {


			if ((haveSelectedPoints || haveSelectedPathGuide || haveSelectedCurve) && item_is_moving) {

				cpr_curve->moveSelectedItem(this->mapToScene(event->pos()), false);
				if (haveSelectedPathGuide) {
					emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);
				}
				
				if (haveSelectedCurve || haveSelectedPoints) {
					//std::cout << "current index : " << cpr_curve->getCurrentIndex() << std::endl;
					emit sigUpdateCPRMod(cpr_curve->getCurvePoints(), cpr_curve->getNumOfControlPoints(), m_slice_type);
					//emit sigUpdateCPRPath(cpr_curve->getIndicatorLeftPoint(), cpr_curve->getIndicatorRightPoint());
					emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);
				}

			}
		}


	}


	QPointF cur_pos = event->localPos();
	//emit sigCoordInfo(cur_pose.x(), cur_pos.y(), this);

	setCoordInfo(cur_pos.x(), cur_pos.y());
	m_coord_box->setVisible(true);

	if (m_mode > 1) {
		QPointF point = _window_to_slice(QPointF(cur_pos.x() * m_cropped_width / this->width(),
			cur_pos.y() * m_cropped_width / this->width()));
		return;
	}

	
	// TODO: emit coord info of cursor location
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

	// TODO: when mouse leaves widget during clicked?
	float max_size = fmax(this->width(), this->height());
	float dx = cur_pos.x() - m_pressed_loc.x();
	float dy = cur_pos.y() - m_pressed_loc.y();
	float dx_norm = dx / max_size;
	float dy_norm = dy / max_size;
	if (m_pressed_type == LEFT_CLICK) {
		
	}
	else if (m_pressed_type == RIGHT_CLICK) {
		if (!(is_horizontal_pressed || is_vertical_pressed)) { // windowing
			//emit sigScreenDragged(dx, dy, m_pressed_type);
			//_setWindowingInfo();
		}
	}
	else if (m_pressed_type == MIDDLE_CLICK) { // panning
		emit sigScreenDragged(-dx_norm, -dy_norm, m_pressed_type);
	}

	m_pressed_loc = cur_pos;
}

void CPRView::mousePressEvent(QMouseEvent* event) {
	is_mouse_pressed = true;

	QGraphicsView::mousePressEvent(event);


	m_pressed_type = _get_clicktype(event);
	m_pressed_loc = event->localPos();
	m_pressed_angle_rad = _get_mouse_angle(m_pressed_loc);
	
	if (m_mode == MPR_CURVE) {
		if (event->button() == Qt::MiddleButton) {
			last_pan_point = event->pos();
			is_panning = true;
			std::cout << " wheel click " << std::endl;
		}


		if (cpr_curve) {
			if (event->button() == Qt::RightButton) {
				std::cout << "pressing right click" << std::endl;
				if (cpr_curve->is_finished()) {
					if (haveSelectedPoints || haveSelectedCurve) {
						cpr_curve->showMenuBar(this->mapToScene(event->pos()));
						if (cpr_curve->AddedNewPoint() || cpr_curve->RemovedExistingPoint()) {
							emit sigUpdateCPRMod(cpr_curve->getCurvePoints(), cpr_curve->getNumOfControlPoints(), m_slice_type);
							//emit sigUpdateCPRPath(cpr_curve->getIndicatorLeftPoint(), cpr_curve->getIndicatorRightPoint());
							emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);

							if (cpr_curve->AddedNewPoint()) {
								cpr_curve->completeAddingNewPoint();
							}
							if (cpr_curve->RemovedExistingPoint()) {
								cpr_curve->completeRemovingExistingPoint();
							}
						}
					}

				}
				
			}
			cpr_curve->unselectAll();
			haveSelectedPoints = cpr_curve->hasSelectedControlPoint();
			haveSelectedCurve = cpr_curve->hasSelectedCurve();
			haveSelectedPathGuide = cpr_curve->hasSelectedPathGuide();
			
			if ((haveSelectedPoints || haveSelectedPathGuide || haveSelectedCurve) && !item_is_moving) {
				
				
				
					cpr_curve->moveSelectedItem(this->mapToScene(event->pos()), true);
					item_is_moving = true;
					//emit sigClicked(m_slice_type, 3);

					//if (haveSelectedCurve || haveSelectedPoints) {
					//	emit sigUpdateCPRMod(cpr_curve->getCurvePoints(), cpr_curve->getNumOfControlPoints(), m_slice_type);
					//	//emit sigUpdateCPRPath(cpr_curve->getIndicatorLeftPoint(), cpr_curve->getIndicatorRightPoint());
					//	emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);
					//}
					//
					//if (haveSelectedPathGuide) {
					//	emit sigClicked(m_slice_type, 2);
					//	emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);
					//
					//}
				
				
				
				
				
			}
		
		}

		

	}
}

void CPRView::mouseReleaseEvent(QMouseEvent* event) {
	if (m_mode < 0)
		return;

	if (!is_mouse_pressed)
		return;

	ClickType released_type = _get_clicktype(event);
	if (released_type != m_pressed_type)
		return;

	if (m_mode == MPR_CURVE) {
		QGraphicsView::mouseReleaseEvent(event);

		if (event->button() == Qt::MiddleButton) {
			is_panning = false;
			std::cout << "wheel released " << std::endl;
		}
		
		if (cpr_curve) {

			if ((haveSelectedPoints || haveSelectedPathGuide || haveSelectedCurve) && item_is_moving) {

				

				if (haveSelectedPathGuide) {
					//emit sigMouseReleased(m_slice_type, 2);
					emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);
				}

				if (haveSelectedCurve || haveSelectedPoints) {
					//emit sigMouseReleased(m_slice_type, 3);
					//std::cout << "current index : " << cpr_curve->getCurrentIndex() << std::endl;
					emit sigUpdateCPRMod(cpr_curve->getCurvePoints(), cpr_curve->getNumOfControlPoints(), m_slice_type);
					//emit sigUpdateCPRPath(cpr_curve->getIndicatorLeftPoint(), cpr_curve->getIndicatorRightPoint());
					emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);
				}
				item_is_moving = false;

			}
		}

	}



	QPointF cur_pos = event->localPos();
	//emit sigCoordInfo(cur_pose.x(), cur_pos.y(), this);

	setCoordInfo(cur_pos.x(), cur_pos.y());
	m_coord_box->setVisible(true);

	if (m_mode > 1) {
		QPointF point = _window_to_slice(QPointF(cur_pos.x() * m_cropped_width / this->width(),
			cur_pos.y() * m_cropped_width / this->width()));
		return;
	}

	

	is_mouse_pressed = false;

}

void CPRView::mouseDoubleClickEvent(QMouseEvent* event) {

	float x, y;
	QPointF cur_pos = event->pos();
	*m_img = m_windowed_img->copy();
	QPointF scene_pos = this->mapToScene(event->pos());
	if (m_mode == MPR_CURVE) {
		if (cpr_curve == nullptr) {
			if (event->button() == Qt::LeftButton) {
				cpr_curve = new GraphicsCurve(this->scene());
				cpr_curve->addPoint(this->mapToScene(event->pos()), false);
				cpr_curve->addScreenPoint(cur_pos);
			}
		}
		else if (cpr_curve->is_finished()) {
			cpr_curve->~GraphicsCurve();
			cpr_curve = new GraphicsCurve(this->scene());
			cpr_curve->addPoint(this->mapToScene(event->pos()), false);
			cpr_curve->addScreenPoint(cur_pos);

		}
		else {
			cpr_curve->addPoint(this->mapToScene(event->pos()), false);
			cpr_curve->addScreenPoint(cur_pos);

			if (event->button() == Qt::RightButton) {
				cpr_curve->drawCurve();
				//cpr_curve->drawInitPath();
				cpr_curve->finish();

				//std::cout << "current index : " << cpr_curve->getCurrentIndex() << std::endl;
				emit sigUpdateCPRMod(cpr_curve->getCurvePoints(), cpr_curve->getNumOfControlPoints(), m_slice_type);
				//emit sigUpdateCPRPath(cpr_curve->getIndicatorLeftPoint(), cpr_curve->getIndicatorRightPoint());
				//emit sigUpdateCPRPath(cpr_curve->getPathPoints(), cpr_curve->getPathLength(), m_slice_type);
				connect(cpr_curve, SIGNAL(sigPathBarAdded()), this, SLOT(slotUpdateCPRPath()));
			}
		}
	}
	else {
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
}

void CPRView::wheelEvent(QWheelEvent* event) {
	if (m_mode < 0)
		return;

	if (is_mouse_pressed)
		return;

	if (m_mode < 2) {
		emit sigWheelChanged(event->angleDelta().y() > 0 ? -1 : 1);
		setCoordInfo(event->posF().x(), event->posF().y());
	}
}

void CPRView::leaveEvent(QEvent* event) {
	// this event does not occur when mouse pressed
	if (m_mode < 0)
		return;

	

	m_coord_box->setVisible(false);
}
