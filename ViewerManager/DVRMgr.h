#pragma once
#define _USE_MATH_DEFINES
#include "Mgr.h"
#include "opengl_utils.h"
#include "GLMgr.h"
#include "TF.h"

#include "../CgipCommon/CgipPlane.h"

#include <QColor>
#include <QObject>

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>
#include <tuple>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "bitmap_image.hpp"
#include "ArcBall.h"

class VIEWERMANAGER_DLL DVRMgr
{
#define PI 3.141592

public:
	DVRMgr();
	~DVRMgr();

	void init_DVR(GLMgr* m_gl_mgr, VolumeMgr* m_vol_mgr);
	void init_position();
	void init_geometry(VROrientationType type = A);

	// functions for manipulating vr
	void zooming(int delta);
	void zoom_by_ratio(float delta);
	void panning(float dx, float dy);
	void rotation(int x, int y, int width, int height);

	void doSculpt(std::vector<int>, std::vector<int>);
	void doVOI(std::vector<int>, std::vector<int>);
	void changeProjection(int type, std::vector<std::vector<std::vector<int>>>);
	void changeColor(std::map<int, std::vector<int>>);
	void resetSculpt();

	// get&set DVR info
	void setDVR();
	inline cgip::CgipPlane getDVR() { return *m_dvr; };
	void initSamplingCount(int cnt);
	void setSamplingCount(int cnt, bool is_downsampling);
	void sampleByPanel(int, int);
	inline void setStartPosition(int x, int y) {

		m_rotate_start_x = x;
		m_rotate_start_y = y;

	}
	void setPanelSize(int width, int height) {
		m_panel_width = width;
		m_panel_height = height;
		m_screen_width = width;
		m_screen_height = height;
		*m_dvr = std::move(cgip::CgipPlane(m_screen_width, m_screen_height));

	}

	void setShaderStatus(int);
	void setLighting(float, float);
	int getMaxIntensity() { return m_max_val; };
	int getMinIntensity() { return m_min_val; };

	DVRPacket getDVRPacket();

private:
	GLMgr* m_gl_mgr = nullptr;
	VolumeMgr* m_volume_mgr = nullptr;
	cgip::CgipPlane* m_dvr = nullptr;
	cgip::CgipVolume* volume_data = nullptr;

	ArcBall* arcball = nullptr;

	std::vector<GLfloat> m_data;

	int m_render_resolution;
	int m_panel_width, m_panel_height;

	// volume info
	int m_width, m_height, m_depth;
	float m_spacingX, m_spacingY, m_spacingZ;
	float m_unit_ray_len;
	glm::vec2 m_sampling_cnt;
	int m_min_val, m_max_val;

	bool m_shader_on = true;
	float m_specular = 0.0;
	float m_brightness = 0.0;

	unsigned char* m_tex_sculpt = nullptr;

	TF* m_tf = nullptr;
	int* dvr_histogram = nullptr;

	// GL
	Texture* tex_tf=nullptr, *sculpt_texture=nullptr;

	//Projection Related
	int m_projection_type;
	float x1, y1, x2, y2, x3, y3, x4, y4;

	// geometry
	float m_num_max, m_screen_dist, m_current_num_max;
	int m_screen_width, m_screen_height;
	float init_pixel_screen_x;
	float init_current_num_max;
	glm::vec2 m_pixel_screen_len;
	glm::vec2 m_center_coordinate;
	glm::vec3 init_PScreen, P_screen, ns, ws, hs;


	int m_rotate_start_x;
	int m_rotate_start_y;
	int rotate_start_width;
	int rotate_start_height;

	glm::mat4 m_camera, m_proj;

	// Matrix
	glm::mat4 trans;
	glm::mat4 shear_mat;
	glm::mat4 m_translate_center;
	glm::mat4 m_translate_center_inverse;
	glm::mat4 current_rotation_matrix;
	glm::mat4 last_rotation_matrix;

	// Functions
	cgip::CgipPlane* sculpt_plane = nullptr;

	// mouse/key event
	bool m_first_pressed = false;
	bool m_is_sculpt = false;
	bool m_is_voi = false;


	/* ====================================================================== */
	/* 						     Private Functions							  */
	/* ====================================================================== */
	
	void _setMaterials();

	void _setSculptTextureData();

	glm::vec3 _get_arcball_pos(int, int, int, int);

	void _HoleFilling(std::vector<cgip::CgipPoint>, CgipShort pval, cgip::CgipPlane* plane);
};