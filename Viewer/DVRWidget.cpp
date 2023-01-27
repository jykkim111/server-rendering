#include "DVRWidget.h"

#include <QLabel>
#include <QLayout>
#include <QDebug>
#include <QPainter>

#include <algorithm>
#include <vector>
#include <iostream>


DVRWidget::DVRWidget(QWidget* parent, const char* name) : QGraphicsView(parent) {
	// Set name
	std::cout << "creating DVRWidget" << std::endl;
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

	this->setMouseTracking(true);
	this->setBackgroundBrush(Qt::black);

	m_rendered_img = this->scene()->addPixmap(m_img->scaled(this->width(), this->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	this->scene()->setSceneRect(-20000, -20000, 40000, 40000);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	m_rendered_img->setPos(0, 0);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
	this->scale(1.0, 1.0);

	m_windowed_img = new QPixmap();

	m_mode = -1;
	is_mouse_pressed = false;

	m_button_layout = new QVBoxLayout(this);
	// zooming
	QVBoxLayout* m_zoom_box = new QVBoxLayout;
	QHBoxLayout* m_otf_btn_box = new QHBoxLayout;
	m_zoom_box->setAlignment(Qt::AlignRight | Qt::AlignTop);
	m_otf_btn_box->setAlignment(Qt::AlignRight | Qt::AlignBottom);

	QPushButton* plus_button = new QPushButton;
	plus_button->setObjectName("btn_3");
	plus_button->setIcon(QIcon("Images/Slice/plus.png"));

	QPushButton* minus_button = new QPushButton;
	minus_button->setObjectName("btn_4");
	minus_button->setIcon(QIcon("Images/Slice/minus.png"));

	QPushButton* otf_button = new QPushButton;
	otf_button->setObjectName("btn_otf");
	otf_button->setIcon(QIcon("Images/Slice/otf.png"));

	connect(plus_button, &QPushButton::clicked, [this] {emit sigWheelChanged(1); });
	connect(minus_button, &QPushButton::clicked, [this] {emit sigWheelChanged(-1); });
	connect(otf_button, &QPushButton::clicked, [this] {emit sigRunOTF(); });


	m_zoom_box->addWidget(plus_button);
	m_zoom_box->addWidget(minus_button);
	m_otf_btn_box->addWidget(otf_button);
	m_button_layout->addLayout(m_zoom_box, Qt::AlignRight | Qt::AlignTop);
	m_button_layout->addLayout(m_otf_btn_box, Qt::AlignRight | Qt::AlignBottom);

	m_fps_box = _setFPSBox(0);
	m_zoomRatio_box = _setZoomBox(2);
	input_zoom_ratio = _setEditBox(2);

	connect(m_zoomRatio_box, SIGNAL(clicked()), this, SLOT(slotZoomRatioClicked()));
	connect(input_zoom_ratio, SIGNAL(editingFinished()), this, SLOT(slotChangeZoomRatio()));

	input_zoom_ratio->setVisible(false);
	m_zoomRatio_box->setVisible(true);

	std::cout << "created DVR widget" << std::endl;

}

void DVRWidget::setMode(int m) {
	m_mode = m;
}


void DVRWidget::setSlice(DVRPacket slice, cgip::CgipPlane plane) {
	m_dvr_packet = slice;
	m_dvr_plane = plane;
	_setScreen();
	_setWindowing();
	m_rendered_img->setPixmap(*m_img);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
}

/* ========================================================================== */
/* 							   Private Functions							  */
/* ========================================================================== */
void DVRWidget::_setSlice() {
	_setScreen();
	_setWindowing();
	m_rendered_img->setPixmap(*m_img);
	this->centerOn(m_img->width() / 2, m_img->height() / 2);
	this->fitInView(0, 0, m_img->width(), m_img->height(), Qt::KeepAspectRatio);
}

void DVRWidget::_setScreen() {
	m_slice_width = m_dvr_plane.getWidth();
	m_slice_height = m_dvr_plane.getHeight();

	m_cropped_width = m_slice_width; //*this->width() / fmax(this->width(), this->height());
	m_cropped_height = m_slice_height; //*this->height() / fmax(this->width(), this->height());

}

void DVRWidget::_setWindowing() {
	int num_channel = m_dvr_plane.getChannel();
	QImage img(m_cropped_width, m_cropped_height, QImage::Format_RGB32);

#pragma omp parallel for
	for (int xy = 0; xy < m_cropped_width * m_cropped_height; xy++) {
		int y = xy / m_cropped_width;
		int x = xy % m_cropped_width;
		QPointF img_coord = _window_to_slice(QPointF(x, y));

		int temp_r, temp_g, temp_b;
		if (num_channel == 1) {
			temp_r = m_dvr_plane.getPixelValue(img_coord.x(), img_coord.y());
			temp_g = temp_r;
			temp_b = temp_r;
		}
		else if (num_channel == 3) {
			temp_r = m_dvr_plane.getPixelValue(img_coord.x(), img_coord.y(), 0);
			temp_g = m_dvr_plane.getPixelValue(img_coord.x(), img_coord.y(), 1);
			temp_b = m_dvr_plane.getPixelValue(img_coord.x(), img_coord.y(), 2);
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
		*m_windowed_img = m_windowed_img->scaledToWidth(this->width(), Qt::SmoothTransformation);
	}
	else {
		*m_windowed_img = m_windowed_img->scaledToHeight(this->height(), Qt::SmoothTransformation);
	}
	*m_img = m_windowed_img->copy();
}

void DVRWidget::_setGuideLine() {
	*m_img = m_windowed_img->copy();

	QPainter* painter = new QPainter(m_img);

	if (m_mode == DVR_SCULPT) {
		for (int i = 1; i < m_points_for_editing.size(); i++) {
			QPointF p1 = m_points_for_editing[i - 1];
			QPointF p2 = m_points_for_editing[i];
			painter->setPen(QPen(Qt::red, 3));
			painter->drawLine(p1, p2);
		}
	}

	if (m_mode == DVR_CROP || m_mode == DVR_VOI) {
		for (int i = 1; i < m_points_for_editing.size(); i++) {
			QPointF p1 = m_points_for_editing[i - 1];
			QPointF p2 = m_points_for_editing[i];
			painter->setPen(QPen(Qt::red, 3));
			painter->drawLine(p1, p2);
		}
		painter->drawLine(m_points_for_editing[3], m_points_for_editing[0]);
	}

	painter->end();

	m_rendered_img->setPixmap(*m_img);

	SAFE_DELETE_OBJECT(painter);
}

QLabel* DVRWidget::_setInfoBox(int i, bool align_top) {
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

ClickableLabel* DVRWidget::_setZoomBox(int i, bool align_top) {
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

QLineEdit* DVRWidget::_setEditBox(int i, bool align_top) {
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

QLabel* DVRWidget::_setFPSBox(int i, bool align_top) {
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

void DVRWidget::setFPSInfo(int fps) {
	QString msg;
	msg = "FPS: " + QString::number(fps);
	m_fps_box->setText(msg);

}

float DVRWidget::round_float(float var) {
	float value = (int)(var * 100 + .5);
	return (float)value / 100;
}

void DVRWidget::setZoomRatioInfo(float ratio) {

	QString zoomRatio;
	zoomRatio = "x" + QString::number(round_float(ratio));
	m_zoomRatio_box->setText(zoomRatio);
	input_zoom_ratio->setText(QString::number(round_float(ratio)));
}


void DVRWidget::slotZoomRatioClicked() {
	m_zoomRatio_box->setVisible(false);
	input_zoom_ratio->setVisible(true);
}

void DVRWidget::slotChangeZoomRatio() {

	if (input_zoom_ratio->text().length() > 0) {
		QString zoomRatio;
		zoomRatio = "x" + input_zoom_ratio->text();
		m_zoomRatio_box->setText(zoomRatio);
		printf("input_ratio: %f", input_zoom_ratio->text().toDouble());
		emit(sigZoomRatioChanged(input_zoom_ratio->text().toDouble()));

	}

	m_zoomRatio_box->setVisible(true);
	input_zoom_ratio->setVisible(false);
}


void DVRWidget::resizeEvent(QResizeEvent* event) {
	if (m_mode < 0)
		m_rendered_img->setPixmap(m_img->scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	else {
		_setSlice();
	}
}

ClickType DVRWidget::_get_clicktype(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		return LEFT_CLICK;
	if (e->button() == Qt::RightButton)
		return RIGHT_CLICK;
	if (e->button() == Qt::MidButton)
		return MIDDLE_CLICK;
}

void DVRWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_mode < 0)
		return;

	// TODO: emit coord info of cursor location
	QPoint cur_pos = event->pos();
	//emit sigCoordInfo(cur_pose.x(), cur_pos.y(), this);

	if ((m_mode == DVR_CROP || m_mode == DVR_VOI) && !m_points.empty()) {
		QPointF first_pos = m_points[0];
		m_points_for_editing.push_back(m_points[0]);
		m_points_for_editing.push_back(QPointF(this->mapToScene(cur_pos).x(), first_pos.y()));
		m_points_for_editing.push_back(this->mapToScene(cur_pos));
		m_points_for_editing.push_back(QPointF(first_pos.x(), this->mapToScene(cur_pos).y()));
		_setGuideLine();
		m_points_for_editing.clear();
		return;
	}

	if (!is_mouse_pressed)
		return;

	if (m_mode == DVR_SCULPT) {
		m_points_for_editing.push_back(this->mapToScene(cur_pos));
		_setGuideLine();
		return;
	}

	// TODO: when mouse leaves widget during clicked?
	float max_size = fmax(this->width(), this->height());
	int min_size = fmin(this->width(), this->height());
	float dx = cur_pos.x() - m_pressed_loc.x();
	float dy = cur_pos.y() - m_pressed_loc.y();
	float dx_norm = dx / max_size;
	float dy_norm = dy / max_size;
	if (m_pressed_type == LEFT_CLICK) { // rotation
		if (m_rotating) {
			emit sigVR_rotate(cur_pos.x(), cur_pos.y(), this->width(), this->height());
		}
		//emit sigScreenDragged(m_pressed_loc.x(), m_pressed_loc.y(), m_pressed_type);
	}
	else if (m_pressed_type == RIGHT_CLICK) { // windowing
		//emit sigScreenDragged(dx, dy, m_pressed_type);
	}
	else if (m_pressed_type == MIDDLE_CLICK) { // panning
		if (m_panning) {
			emit sigVR_pan(dx, dy);
		}
		//emit sigScreenDragged(dx, dy, m_pressed_type);
	}

	m_pressed_loc = cur_pos;
}

void DVRWidget::mousePressEvent(QMouseEvent* event) {
	if (m_mode < 0)
		return;

	if (is_mouse_pressed)
		return;

	is_mouse_pressed = true;
	m_pressed_type = _get_clicktype(event);
	m_pressed_loc = event->pos();
	m_initial_coord = event->localPos();
	int min_size = fmin(this->width(), this->height());

	if (m_mode == DVR_CROP || m_mode == DVR_VOI) {
		if (m_points.empty()) {
			m_points.push_back(this->mapToScene(m_pressed_loc));
		}
		else {
			m_points.push_back(QPointF(this->mapToScene(m_pressed_loc).x(), m_points[0].y()));
			m_points.push_back(this->mapToScene(m_pressed_loc));
			m_points.push_back(QPointF(m_points[0].x(), this->mapToScene(m_pressed_loc).y()));

			std::vector<int> points_x, points_y;
			if (m_slice_width == m_slice_height) {
				for (int i = 0; i < m_points.size(); i++) {
					QPointF cur = QPointF(m_points[i].x(),
						m_points[i].y() );
					points_x.push_back(cur.x());
					points_y.push_back(cur.y());
				}
			}
			else {
				for (int i = 0; i < m_points.size(); i++) {
					QPointF cur = QPointF(m_points[i].x(),
						m_points[i].y());
					points_x.push_back(cur.x());
					points_y.push_back(cur.y());
				}
			}


			m_points.clear();
			if (m_mode == DVR_CROP)
				emit sigCrop(points_x, points_y);
			else if (m_mode == DVR_VOI)
				emit sigVOI(points_x, points_y);
		}
		return;
	}

	if (m_mode == NONE) {
		if (m_pressed_type == LEFT_CLICK) { // rotation
			emit sigMouseClicked(m_pressed_loc.x(), m_pressed_loc.y(), this->width(), this->height(), m_pressed_type);
			m_rotating = true;
		}
		else if (m_pressed_type == MIDDLE_CLICK) {
			emit sigMouseClicked(0, 0, this->width(), this->height(), m_pressed_type);
			m_panning = true;
		}
	}
}

void DVRWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (m_mode < 0)
		return;

	if (!is_mouse_pressed)
		return;

	QPoint cur_pos = event->pos();

	if (m_mode == DVR_SCULPT) {
		std::vector<int> points_x, points_y;
		if (m_slice_width == m_slice_height) {
			for (int i = 0; i < m_points_for_editing.size(); i++) {
				QPointF cur = QPointF(m_points_for_editing[i].x(),
					m_points_for_editing[i].y());
				points_x.push_back(cur.x());
				points_y.push_back(cur.y());
			}
		}
		else {
			for (int i = 0; i < m_points_for_editing.size(); i++) {
				QPointF cur = QPointF(m_points_for_editing[i].x(),
					m_points_for_editing[i].y());
				points_x.push_back(cur.x());
				points_y.push_back(cur.y());
			}
		}

		emit sigSculpt(points_x, points_y);

		m_points_for_editing.clear();
		_setGuideLine();
	}

	if (m_mode == NONE) {
		float dx = cur_pos.x() - m_pressed_loc.x();
		float dy = cur_pos.y() - m_pressed_loc.y();

		if (m_pressed_type == LEFT_CLICK) { // rotation
			m_rotating = false;

			emit sigMouseReleased();
		}
		else if (m_pressed_type == MIDDLE_CLICK) { // panning
			m_panning = false;
			emit sigMouseReleased();
		}
	}

	m_pressed_loc = cur_pos;

	ClickType released_type = _get_clicktype(event);
	if (released_type != m_pressed_type)
		return;

	is_mouse_pressed = false;

}

void DVRWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	if (m_mode < 0 || m_mode >0)
		return;

	if (is_mouse_pressed)
		return;

	emit sigDoubleClicked(_get_clicktype(event));
}

void DVRWidget::wheelEvent(QWheelEvent* event) {
	if (m_mode < 0)
		return;

	if (is_mouse_pressed)
		return;

	//emit sigWheelChanged(event->angleDelta().y() > 0 ? 1 : -1);
}

void DVRWidget::leaveEvent(QEvent* event) {
	// this event does not occur when mouse pressed
	if (m_mode < 0)
		return;
}
