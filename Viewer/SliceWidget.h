#pragma once

#include <QWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QResizeEvent>
#include <QString>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSlider>
#include "ClickableLabel.h"
#include "CPRCurve.h"
#include "MPR_GuideLine.h"

#include "../CgipCommon/CgipPoint.h"
#include "../ViewerManager/global.h"

#define PI 3.1415926535897
#define DELTA 5
#define EPSILON 0.000001



class SliceWidget : public QGraphicsView
{
	Q_OBJECT

public:
	SliceWidget(QWidget* parent, const char *name=NULL, SliceType slice_type=AXIAL_SLICE);

	// main flow methods
	void setSlice(SlicePacket slice, cgip::CgipPlane);
	void setGuideLine(SlicePacket slice);
	void setGuideLine(SlicePacket slice, cgip::CgipPlane);

	// set & get methods
	void setMode(int m);
	inline int getMode() { return m_mode; }
	void toggleDrawGuideLine();
	inline int getWidth() { return this->width(); }
	inline int getHeight() { return this->height(); }
	inline float getSliceType() { return this->m_slice_type; }
	inline void setWLRange(std::tuple<int, int> v) { int l, h; std::tie(l, h) = v; m_wl_slider->setRange(l, h); }
	inline void setWWRange(std::tuple<int, int> v) { int l, h; std::tie(l, h) = v; m_ww_slider->setRange(l, h); }
	inline void setWL(int v) { m_wl_slider->setValue(v); }
	inline void setWW(int v) { m_ww_slider->setValue(v); }

	void setCoordInfo(float, float);
	void setSliceInfo(float x=0, float y=0);
	void setFPSInfo(int);
	void setZoomRatioInfo(float ratio);
	float round_float(float x);


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
	void sigResize(int, int, SliceWidget*);
	void sigGuideLineMoved(bool, bool, float, float);
	void sigGuideLineRotated(float);
	void sigGuideLineThickness(float, bool, bool);
	void sigScreenDragged(float, float, ClickType);
	void sigMouseReleased(float, float, ClickType);
	void sigDoubleClicked(float, float, ClickType);
	void sigClicked(float, float, ClickType);
	void sigWheelChanged(float);
	void sigZoomRatioChanged(float);
	void sigUpdateCPRMod(std::vector<QPointF>, int);
	//void sigUpdateCPRPath(QPointF, QPointF);
	void sigUpdateCPRPath(std::vector<QPointF>, float);
	void sigCPRInit();

	void sigReqFullScreen();
	void sigReqCPRScreen();

public slots:
	//void slotResize(int w, int h);
	void slotChangeGuideLine();
	void slotChangeZoomRatio();
	void slotZoomRatioClicked();

private:
	// MPR Info
	SlicePacket m_slice_packet;
	cgip::CgipPlane m_mpr_plane;

	QPixmap *m_img=nullptr, *m_windowed_img=nullptr;
	QGraphicsPixmapItem* m_rendered_img = nullptr;
	MPR_GuideLine* mprGuideLine = nullptr;

	CPRCurve *cpr_curve = nullptr;
	QLabel* m_coord_box;
	QLabel* m_fps_box;
	QLabel* m_slice_num_box;
	QLabel* m_func_info_box;
	QLabel* m_windowing_box;
	ClickableLabel* m_zoom_ratio_box;
	QLineEdit* input_zoom_ratio;

	QVBoxLayout* m_button_layout;
	QSlider* m_wl_slider, * m_ww_slider;

	SliceType m_slice_type;
	float m_horizontal_angle_rad;
	QPointF m_center;
	QPoint m_center_origin;
	QPointF m_pressed_loc;
	float m_pressed_angle_rad;
	int m_slice_width, m_slice_height, m_cropped_width, m_cropped_height;
	int m_width_scaled, m_height_scaled;


	float fTranslate, marginCenter, marginTranslate, marginRotate;

	QGraphicsLineItem guideLines[4];
	QBrush brush_forHorizontalGuideLine;
	QBrush brush_forVerticalGuideLine;

	QPen pen_forNormalBox = QPen(Qt::green, 1, Qt::SolidLine);
	QBrush brush_forNormalBox = QBrush(QColor(20, 200, 20, 255));
	QPen pen_forPath = QPen(Qt::green, 2, Qt::SolidLine);

	QPen pen_forIndicatorCircle = QPen(Qt::blue, 1, Qt::SolidLine);
	QBrush brush_forIndicatorCircle = QBrush(QColor(Qt::blue));

	int m_mode;
	bool is_draw_guideline;
	bool m_cpr_on = false;
	bool is_horizontal_pressed, is_vertical_pressed;
	bool is_horizontal_mouseover, is_vertical_mouseover;
	bool is_mouse_pressed;
	ClickType m_pressed_type;

	const float m_center_radius = 5.0;
	int m_window_level, m_window_width;

	/* ====================================================================== */
	/* 						     Private Functions							  */
	/* ====================================================================== */
	inline float _rad(float degree) { return degree * (PI / 180); }
	inline float _deg(float radiance) { return radiance * (180.f / PI); }
	void _setSlice();
	void _setScreen();
	void _setPadding();
	void _setWindowing();
	void _setCPRCurve();
	//int _applyWindowing(int v);
	void _setGuideLine(float center_radius=20, float thickness=1);
	void _setNewGuideLine(float center_radius = 20, float thickness = 1);
	QLabel* _setInfoBox(int i, bool align_top=true);
	ClickableLabel* _setZoomBox(int i, bool align_top = true);
	QLineEdit* _setEditBox(int i, bool align_top = true);
	QLabel* _setFPSBox(int i, bool align_top = true);
	void _setWindowingInfo();


	ClickType _get_clicktype(QMouseEvent *e);
	float _get_mouse_angle(QPointF p);

	inline QPointF _window_to_slice(QPointF window, float ratio=1.0) {
		return QPointF((m_slice_width / 2 - m_cropped_width / 2 + window.x()) * ratio, (m_slice_height / 2 - m_cropped_height / 2 + window.y()) * ratio);
	}
	inline QPointF _slice_to_window(QPointF slice, float ratio=1.0) {
		return QPointF(-m_slice_width / 2 + m_cropped_width / 2 + slice.x() * ratio, -m_slice_height / 2 + m_cropped_height / 2 + slice.y() * ratio);
	}
};
