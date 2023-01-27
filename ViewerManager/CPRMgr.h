#pragma once

#include "viewermanager.h"

#include <QObject>
#include <QColor>

#include <tuple>
#include <cmath>

#include "GLMgr.h"
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

using namespace cgip;

class VIEWERMANAGER_DLL CPRMgr : public QObject
{
	Q_OBJECT
public:
	CPRMgr();
	~CPRMgr();

	void change_position(SliceType slice_type, int axial_loc=-1);
	void initSamplingCount(int w_cnt, int h_cnt, int c);
	void setSamplingCount(int w_cnt, int h_cnt, int c);
	
	//Set Volume Manager
	void init_CPR(GLMgr* gl_mgr, VolumeMgr* vol_mgr, SliceType slice_type);


	//
	CPRCurvePacket getCurvePacket(SliceType);
	//SlicePacket getPathPacket();

	void cpr_curve(SliceType slice_type, std::vector<float> point_x, std::vector<float> point_y, int numOfPoints);
	void cpr_path(SliceType slice_type, std::vector<float> point_x, std::vector<float> point_y, float pathLength);

	CgipPoint mapViewTo3DPointF(SliceType slice_type, float x, float y, bool for_curve);

	bool is_outsideVolume(cgip::CgipPoint p);
	void setCPR(SliceType slice_type);
	void setCPRPath(SliceType slice_type);
	
	inline CgipPlane getCPR() { return *m_cpr; }
	inline CgipPlane getCPRPath() { return *m_cpr_path; }

private:
	float _get_projection(CgipVector a, CgipVector b);

	// private functions
	CgipPlane _setCPR(int slice_type_num,
		CgipPoint& topleft_p,
		CgipVector w_dir,
		CgipVector h_dir,
		CgipVector n_dir,
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

	// private functions
	CgipPlane _setCPRPath(int slice_type_num,
		CgipPoint& staringPoint,
		CgipVector indicator_dir,
		CgipVector n_dir,
		int w_cnt,
		int h_cnt,
		int n_cnt,
		float path_sampling_len,
		float n_sampling_len,
		bool thickness,
		int window_width,
		int window_level,
		bool is_inverse);

	

	float _get_dist_next_slice(float d, float cur_loc, float spacing);

	CgipPlane* m_cpr = nullptr;
	CgipPlane* m_cpr_path = nullptr;

	GLMgr* m_gl_mgr = nullptr;
	VolumeMgr* m_vol_mgr = nullptr;
	
	CPR_COORD* m_pathPoints;
	Texture* cpr_pathPoints = nullptr;

	CPR_COORD* m_curvePoints;
	Texture* cpr_curvePoints = nullptr;

	float SLICE_MARGIN = 30;

	int m_axial_window_width, m_axial_window_level;
	int m_sagittal_window_width, m_sagittal_window_level;
	int m_coronal_window_width, m_coronal_window_level;
	int m_thickness_mode = 0; // 0:avg, 1:min, 2:max
	int m_axial_thickness, m_coronal_thickness, m_sagittal_thickness;

	//float m_axial_size, m_coronal_size, m_sagittal_size;
	CgipPoint m_axis_center_forPath, m_axis_center_forCurve;
	CgipPoint m_axial_topleft_forCurve, m_coronal_topleft_forCurve, m_sagittal_topleft_forCurve;
	CgipPoint m_axial_topleft_forPath, m_coronal_topleft_forPath, m_sagittal_topleft_forPath;
	CgipPoint m_path_starting_pt;
	
	CgipVector m_axial_w_dir, m_axial_h_dir, m_axial_n_dir;
	CgipVector m_coronal_w_dir, m_coronal_h_dir, m_coronal_n_dir;
	CgipVector m_sagittal_w_dir, m_sagittal_h_dir, m_sagittal_n_dir;
	CgipVector m_indicator_dir;
	

	float m_axial_size, m_coronal_size, m_sagittal_size;
	float init_axial_size, init_coronal_size, init_sagittal_size;
	float m_axial_w_sampling_len, m_axial_h_sampling_len;
	float m_coronal_w_sampling_len, m_coronal_h_sampling_len;
	float m_sagittal_w_sampling_len, m_sagittal_h_sampling_len;
	float m_n_sampling_len;
	float m_path_sampling_len;
	float m_path_length;
	int m_w_sampling_cnt, m_h_sampling_cnt;

	bool m_axial_inverse = false;
	bool m_sagittal_inverse = false;
	bool m_coronal_inverse = false;

};