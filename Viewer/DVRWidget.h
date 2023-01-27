#pragma once

#include <QWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QResizeEvent>
#include <QString>
#include <QPushButton>
#include <QHBoxLayout>
#include "ClickableLabel.h"

#include "../CgipCommon/CgipPoint.h"
#include "../ViewerManager/global.h"
#include <QtWidgets>

#define PI 3.1415926535897
#define DELTA 5
#define EPSILON 0.000001

class DVRWidget : public QGraphicsView
{
	Q_OBJECT

public:
	DVRWidget(QWidget* parent, const char* name = NULL);

	// main flow methods
	void setSlice(DVRPacket slice, cgip::CgipPlane);

	// set & get methods
	void setMode(int m);
	inline int getMode() { return m_mode; }

	inline int getWidth() { return this->width(); }
	inline int getHeight() { return this->height(); }

protected:
	// Mouse Events
	void resizeEvent(QResizeEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mouseDoubleClickEvent(QMouseEvent*) override;
	void wheelEvent(QWheelEvent*) override;
	void leaveEvent(QEvent*) override;

signals:
	void sigResize(int, int, DVRWidget*);
	void sigMouseClicked(float, float, int, int, ClickType);
	//void sigScreenDragged(float, float, ClickType);
	void sigDoubleClicked(ClickType);
	void sigWheelChanged(float);
	//void sigMouseReleased(float, float, ClickType);
	void sigMouseReleased(void);
	void sigVR_rotate(float, float, int, int);
	void sigVR_pan(float, float);
	void sigSculpt(std::vector<int>, std::vector<int>);
	void sigCrop(std::vector<int>, std::vector<int>);
	void sigVOI(std::vector<int>, std::vector<int>);
	void sigRunOTF();
	void sigZoomRatioChanged(float);


public slots:
	void slotChangeZoomRatio();
	void slotZoomRatioClicked();

public:
	void setFPSInfo(int);
	void setZoomRatioInfo(float);
	float round_float(float x);



private:
	// dvr info
	DVRPacket m_dvr_packet;
	cgip::CgipPlane m_dvr_plane;

	QPixmap* m_img, * m_windowed_img;
	QGraphicsPixmapItem* m_rendered_img = nullptr;
	QLabel* m_fps_box;
	ClickableLabel* m_zoomRatio_box;
	QLineEdit* input_zoom_ratio;
	QVBoxLayout* m_button_layout;

	SliceType m_slice_type;
	float m_horizontal_angle_rad;
	QPointF m_center, m_center_origin;
	QPoint m_pressed_loc;
	QPointF m_initial_coord;
	float m_pressed_angle_rad;
	int m_slice_width, m_slice_height, m_cropped_width, m_cropped_height;

	int m_mode;
	bool is_draw_guideline;
	bool is_horizontal_pressed, is_vertical_pressed;
	bool is_horizontal_mouseover, is_vertical_mouseover;
	bool is_mouse_pressed;
	bool m_rotating = false;
	bool m_panning = false;
	ClickType m_pressed_type;

	const float m_center_radius = 5.0;

	std::vector<QPointF> m_points;
	std::vector<QPointF> m_points_for_editing;

	/* ====================================================================== */
	/* 						     Private Functions							  */
	/* ====================================================================== */
	inline float _rad(float degree) { return degree * (PI / 180); }
	inline float _deg(float radiance) { return radiance * (180.f / PI); }
	void _setSlice();
	void _setScreen();
	void _setWindowing();
	void _setGuideLine();
	ClickableLabel* _setZoomBox(int, bool align_top = true);
	QLineEdit* _setEditBox(int, bool align_top = true);
	QLabel* _setInfoBox(int, bool align_top = true);
	QLabel* _setFPSBox(int i, bool align_top=true);


	//void _setCoordInfoBox();
	//void setCoordInfoBox();
	ClickType _get_clicktype(QMouseEvent* e);

	inline QPointF _window_to_slice(QPointF window, float ratio = 1.0) {
		return QPointF((m_slice_width / 2 - m_cropped_width / 2 + window.x()) * ratio, (m_slice_height / 2 - m_cropped_height / 2 + window.y()) * ratio);
	}
	inline QPointF _slice_to_window(QPointF slice, float ratio = 1.0) {
		return QPointF(-m_slice_width / 2 + m_cropped_width / 2 + slice.x() * ratio, -m_slice_height / 2 + m_cropped_height / 2 + slice.y() * ratio);
	}
};
