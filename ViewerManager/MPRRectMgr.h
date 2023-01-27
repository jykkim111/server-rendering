#pragma once

#include "viewermanager.h"

#include <QObject>
#include <QColor>

#include <tuple>
#include <cmath>

#include "GLMgr.h"
#include "VolumeMgr.h"
#include "opengl_utils.h"

#include "../CgipCommon/CgipVector.h"
#include "../CgipCommon/CgipPlane.h"
#include "../CgipCommon/CgipPoint.h"

#include "global.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "bitmap_image.hpp"

#define PI 3.1415926535897
#define EPSILON 0.00005


class VIEWERMANAGER_DLL MPRRectMgr : public QObject
{
	Q_OBJECT
public:
	MPRRectMgr();
	~MPRRectMgr();

	// Set Volume Manager and Mask Manager
	void init_MPR(GLMgr* gl_mgr, VolumeMgr* vol_mgr);
	void init_position(int axial_loc = -1);
	void init_position(SliceType slice_type, int axial_loc = -1);

	// set slice sampling counts
	void initSamplingCount(int w_cnt, int h_cnt, int c);
	void setSamplingCount(int w_cnt, int h_cnt, int c, bool is_downsampling);
	void setOutputImageSize(int image_width, int image_height, int num_channel) {
		m_output_width = image_width;
		m_output_height = image_height;
		m_panel_width = image_width;
		m_panel_height = image_height;
		m_panel_wh_ratio = 1.0 * m_panel_width / m_panel_height;
		*m_axial = std::move(cgip::CgipPlane(m_output_width, m_output_height, num_channel));
		*m_sagittal = std::move(cgip::CgipPlane(m_output_width, m_output_height, num_channel));
		*m_coronal = std::move(cgip::CgipPlane(m_output_width, m_output_height, num_channel));
	}

	// get slice info
	std::tuple<int, int, float, float, cgip::CgipVector, cgip::CgipVector, cgip::CgipPoint> getAxialInfo();
	std::tuple<int, int, float, float, cgip::CgipVector, cgip::CgipVector, cgip::CgipPoint> getCoronalInfo();
	std::tuple<int, int, float, float, cgip::CgipVector, cgip::CgipVector, cgip::CgipPoint> getSagittalInfo();

	SlicePacket getSlicePacket(SliceType type);

	// set & get slice planes
	void MPRRectMgr::setAxial();
	void MPRRectMgr::setCoronal();
	void MPRRectMgr::setSagittal();
	inline cgip::CgipPlane getAxial() { return *m_axial; }
	inline cgip::CgipPlane getCoronal() { return *m_coronal; }
	inline cgip::CgipPlane getSagittal() { return *m_sagittal; }
	inline cgip::CgipPlane getRaw() { return *m_raw; }

	void setRawMPR(SliceType);

	// functions for manipulating slice planes
	int move_slice(SliceType slice_type, VolumeMgr* vol_mgr, float x, float y, bool h, bool v);
	int scroll_slice(SliceType slice_type, VolumeMgr* vol_mgr, float d);
	int align_center(SliceType slice_type, VolumeMgr* vol_mgr, float x, float y);
	int zoom_slice(SliceType slice_type, VolumeMgr* vol_mgr, float d);
	int zoom_slice_ratio(SliceType slice_type, VolumeMgr* vol_mgr, float ratio);
	int slice_panning(SliceType slice_type, VolumeMgr* vol_mgr, float dx, float dy);
	int rotate_slice(SliceType slice_type, float a);
	int windowing(SliceType slice_type, float dx, float dy);
	int thickness(SliceType slice_type, int v);
	inline void thickness_mode(int v) { m_thickness_mode = v; }
	int inverse(SliceType, bool v);

	// get line info
	std::tuple<float, float, float> getSliceLineInfo(SliceType slice_type);
	std::tuple<float, float> getThicknessInfo(SliceType slice_type);

	// get 3d coord info
	cgip::CgipPoint getCoordInfo(SliceType, float, float);
	std::tuple<int, int, int> getSliceNumInfo(SliceType);
	std::tuple<int, int> getPanningInfo(SliceType);
	inline void setSliceNumOffset(int n) { m_slice_num_offset = n; }

private:
	GLMgr* m_gl_mgr = nullptr;
	VolumeMgr* m_vol_mgr = nullptr;

	// MPR slice planes
	cgip::CgipPlane* m_axial = nullptr;
	cgip::CgipPlane* m_coronal = nullptr;
	cgip::CgipPlane* m_sagittal = nullptr;
	cgip::CgipPlane* m_raw = nullptr;

	// slice plane info
	float SLICE_MARGIN = 30;
	//float m_axial_size, m_coronal_size, m_sagittal_size;
	cgip::CgipPoint m_axis_center_pt;
	cgip::CgipPoint m_axial_topleft_pt, m_coronal_topleft_pt, m_sagittal_topleft_pt;
	cgip::CgipVector m_axial_w_dir, m_axial_h_dir, m_axial_n_dir;
	cgip::CgipVector m_coronal_w_dir, m_coronal_h_dir, m_coronal_n_dir;
	cgip::CgipVector m_sagittal_w_dir, m_sagittal_h_dir, m_sagittal_n_dir;

	float m_axial_size, m_coronal_size, m_sagittal_size;
	float init_axial_size, init_coronal_size, init_sagittal_size;
	float m_axial_w_sampling_len, m_axial_h_sampling_len;
	float m_coronal_w_sampling_len, m_coronal_h_sampling_len;
	float m_sagittal_w_sampling_len, m_sagittal_h_sampling_len;
	int m_w_sampling_cnt, m_h_sampling_cnt;
	int m_output_width, m_output_height;
	int m_panel_width, m_panel_height;
	float m_panel_wh_ratio;

	//float m_axial_w_sampling_len, m_axial_h_sampling_len;
	//float m_coronal_w_sampling_len, m_coronal_h_sampling_len;
	//float m_sagittal_w_sampling_len, m_sagittal_h_sampling_len;
	//int m_axial_w_sampling_cnt, m_axial_h_sampling_cnt;
	//int m_coronal_w_sampling_cnt, m_coronal_h_sampling_cnt;
	//int m_sagittal_w_sampling_cnt, m_sagittal_h_sampling_cnt;
	float m_angle_on_axial, m_angle_on_coronal, m_angle_on_sagittal;
	int m_slice_num_offset = 0;
	int m_axial_window_width, m_axial_window_level;
	int m_sagittal_window_width, m_sagittal_window_level;
	int m_coronal_window_width, m_coronal_window_level;
	int m_thickness_mode = 0; // 0:avg, 1:min, 2:max
	int m_axial_thickness, m_coronal_thickness, m_sagittal_thickness;

	bool m_axial_inverse = false;
	bool m_sagittal_inverse = false;
	bool m_coronal_inverse = false;

	// private functions
	cgip::CgipPlane _setMPR(cgip::CgipPoint& topleft_p,
		cgip::CgipVector w_dir,
		cgip::CgipVector h_dir,
		cgip::CgipVector n_dir,
		int w_cnt,
		int h_cnt,
		int n_cnt,
		float w_len,
		float h_len,
		float n_len,
		bool thickness,
		int window_width,
		int window_level,
		bool is_inverse);
	void _resetTopLeftPt();
	bool _is_outside(VolumeMgr* vol_mgr, cgip::CgipPoint p);
	float _get_projection(cgip::CgipVector, cgip::CgipVector);
	float _add_angle(float a, float b);
	float _get_dist_next_slice(float d, float cur_loc, float spacing);
	void getViewingPlane(
		cgip::CgipPoint& topleft_p,
		cgip::CgipVector w_dir,
		cgip::CgipVector h_dir,
		int w_cnt,
		int h_cnt,
		float w_len,
		float h_len);
	std::tuple<cgip::CgipPoint, cgip::CgipPoint> _find_intersection_point(cgip::CgipPoint center, cgip::CgipVector dir);
};
