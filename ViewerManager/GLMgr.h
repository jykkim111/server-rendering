#pragma once

#include "Mgr.h"
#include "opengl_utils.h"
#include "VolumeMgr.h"
#include "MaskMgr.h"

#include "../CgipCommon/CgipPlane.h"

#include <QColor>
#include <QObject>
#include <QMessageBox>

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>
#include <tuple>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "bitmap_image.hpp"

class VIEWERMANAGER_DLL GLMgr
{
#define PI 3.141592

public:
	GLMgr();
	~GLMgr();

	void init_GL(VolumeMgr* vol_mgr, MaskMgr* mask_mgr);

	// setter
	void setDataTexture();
	void setMaskTexture();
	void setDownsampledDataTexture(int len);
	void setDownsampledMaskTexture(int len);
	void setVert(int sampling_cnt_x, int sampling_cnt_y);
	void setMaskShaderParam(RenderingType);

	// getter
	GLFWwindow* getGLWindow() { return m_gl_window; }
	int getDataTextureId() { return data_texture->ID; }
	int getMaskTextureId() { return mask_texture->ID; }
	int getVAOId() { return m_vao->ID; }
	Shader* getVRShader() { return m_vr_shader; }
	Shader* getMPRShader() { return m_mpr_shader; }
	Shader* getCPRShader() { return m_cpr_shader; }
	Shader* getCPRPathShader() { return m_cpr_path_shader; }

	int getMinVal() { return m_min_val; }
	int getMaxVal() { return m_max_val; }
	float getSpacingX() { return m_spacingX; }
	float getSpacingY() { return m_spacingY; }
	float getSpacingZ() { return m_spacingZ; }
	int getWidth() { return m_width; }
	int getHeight() { return m_height; }
	int getDepth() { return m_depth; }
	int getWindowWidth() { return m_window_width; }
	int getWindowCenter() { return m_window_center; }
	float getWidthMM() { return widthMM; }
	float getHeightMM() { return heightMM; }
	float getDepthMM() { return depthMM; }

	void setImageSize(int panel_width, int panel_height) {
		m_screen_width = panel_width;
		m_screen_height = panel_height;
	}

	VolumeMgr* m_volume_mgr = nullptr;
	MaskMgr* m_mask_mgr = nullptr;

private:
	// Data
	int m_width, m_height, m_depth;
	int m_min_val, m_max_val;
	float m_spacingX, m_spacingY, m_spacingZ;
	int m_window_width, m_window_center;
	float widthMM, heightMM, depthMM;

	// GL
	Shader* m_vr_shader = nullptr;
	Shader* m_mpr_shader = nullptr;
	Shader* m_cpr_shader = nullptr;
	Shader* m_cpr_path_shader = nullptr;

	int m_screen_size=1024;
	int m_screen_width, m_screen_height;

	VAO* m_vao = nullptr;
	Texture* data_texture = nullptr, * mask_texture = nullptr;
	GLFWwindow* m_gl_window = nullptr;

	/* ====================================================================== */
	/* 						     Private Functions							  */
	/* ====================================================================== */
};