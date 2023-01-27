#include "CPRMgr.h"


CPRMgr::CPRMgr() {

	m_cpr = new CgipPlane();
	m_cpr_path = new CgipPlane();
}


CPRMgr::~CPRMgr() {
	SAFE_DELETE_OBJECT(m_cpr);
	SAFE_DELETE_OBJECT(m_cpr_path);
}

void CPRMgr::initSamplingCount(int w_cnt, int h_cnt, int c) {
	m_w_sampling_cnt = w_cnt;
	m_h_sampling_cnt = h_cnt;

	*m_cpr = std::move(cgip::CgipPlane(w_cnt, h_cnt, c));
	*m_cpr_path = std::move(cgip::CgipPlane(w_cnt, h_cnt, c));

}

void CPRMgr::setSamplingCount(int w_cnt, int h_cnt, int c) {
	initSamplingCount(w_cnt, h_cnt, c);
	m_axial_w_sampling_len = m_axial_size / (w_cnt - 1);
	m_axial_h_sampling_len = m_axial_size / (h_cnt - 1);
	m_coronal_w_sampling_len = m_coronal_size / (w_cnt - 1);
	m_coronal_h_sampling_len = m_coronal_size / (h_cnt - 1);
	m_sagittal_w_sampling_len = m_sagittal_size / (w_cnt - 1);
	m_sagittal_h_sampling_len = m_sagittal_size / (h_cnt - 1);

}

void CPRMgr::init_CPR(GLMgr* gl_mgr, VolumeMgr* vol_mgr, SliceType slice_type) {
	m_gl_mgr = gl_mgr;
	m_vol_mgr = vol_mgr;

	//set windowing
	int min_val = m_gl_mgr->getMinVal();
	int max_val = m_gl_mgr->getMaxVal();
	int default_window_center = m_gl_mgr->getWindowCenter();
	int default_window_width = m_gl_mgr->getWindowWidth();
	if (default_window_width == 0) {
		default_window_center = (min_val + max_val) / 2;
		default_window_width = max_val - min_val;
		if (default_window_width < 10)
			default_window_width = 10;
	}
	m_axial_window_level = default_window_center;
	m_axial_window_width = default_window_width;
	m_sagittal_window_level = default_window_center;
	m_sagittal_window_width = default_window_width;
	m_coronal_window_level = default_window_center;
	m_coronal_window_width = default_window_width;	

	m_pathPoints = SAFE_ALLOC_1D(CPR_COORD, m_w_sampling_cnt);
	std::memset(m_pathPoints, 0, sizeof(CPR_COORD) * m_w_sampling_cnt);

	m_curvePoints = SAFE_ALLOC_1D(CPR_COORD, m_w_sampling_cnt);
	std::memset(m_curvePoints, 0, sizeof(CPR_COORD) * m_w_sampling_cnt);

	if (cpr_pathPoints) {
		SAFE_DELETE_OBJECT(cpr_pathPoints);
	}

	if (cpr_curvePoints) {
		SAFE_DELETE_OBJECT(cpr_curvePoints);
	}

	cpr_pathPoints = new Texture(m_pathPoints, m_w_sampling_cnt, 2);
	cpr_curvePoints = new Texture(m_curvePoints, m_w_sampling_cnt, 3);

	change_position(slice_type);
}


CPRCurvePacket CPRMgr::getCurvePacket(SliceType type) {
	CPRCurvePacket packet;
	packet.center_point = { m_axis_center_forCurve.x() ,  m_axis_center_forCurve.y(),  m_axis_center_forCurve.z() };

	switch (type) {
	case AXIAL_SLICE:
		packet.lefttop_point = { m_axial_topleft_forCurve.x() ,  m_axial_topleft_forCurve.y(),  m_axial_topleft_forCurve.z() };
		packet.normal_vector = { m_axial_n_dir.x() ,  m_axial_n_dir.y(),  m_axial_n_dir.z() };
		packet.width_direction_vetor = { m_axial_w_dir.x() ,  m_axial_w_dir.y(),  m_axial_w_dir.z() };
		packet.height_direction_vector = { m_axial_h_dir.x() ,  m_axial_h_dir.y(),  m_axial_h_dir.z() };
		packet.pixel_space = { m_axial_w_sampling_len, m_axial_h_sampling_len };
		packet.m_zoom_factor = init_axial_size / m_axial_size;

		break;
	case CORONAL_SLICE:
		packet.lefttop_point = { m_coronal_topleft_forCurve.x() ,  m_coronal_topleft_forCurve.y(),  m_coronal_topleft_forCurve.z() };
		packet.normal_vector = { m_coronal_n_dir.x() ,  m_coronal_n_dir.y(),  m_coronal_n_dir.z() };
		packet.width_direction_vetor = { m_coronal_w_dir.x() ,  m_coronal_w_dir.y(),  m_coronal_w_dir.z() };
		packet.height_direction_vector = { m_coronal_h_dir.x() ,  m_coronal_h_dir.y(),  m_coronal_h_dir.z() };
		packet.pixel_space = { m_coronal_w_sampling_len, m_coronal_h_sampling_len };
		packet.m_zoom_factor = init_coronal_size / m_coronal_size;
		break;
	case SAGITTAL_SLICE:
		packet.lefttop_point = { m_sagittal_topleft_forCurve.x() ,  m_sagittal_topleft_forCurve.y(),  m_sagittal_topleft_forCurve.z() };
		packet.normal_vector = { m_sagittal_n_dir.x() ,  m_sagittal_n_dir.y(),  m_sagittal_n_dir.z() };
		packet.width_direction_vetor = { m_sagittal_w_dir.x() ,  m_sagittal_w_dir.y(),  m_sagittal_w_dir.z() };
		packet.height_direction_vector = { m_sagittal_h_dir.x() ,  m_sagittal_h_dir.y(),  m_sagittal_h_dir.z() };
		packet.pixel_space = { m_sagittal_w_sampling_len, m_sagittal_h_sampling_len };
		packet.m_zoom_factor = init_sagittal_size / m_sagittal_size;
		break;
	}

	return packet;
}





void CPRMgr::change_position(SliceType slice_type, int axial_loc) {



	float width_mm = m_gl_mgr->getWidthMM();
	float height_mm = m_gl_mgr->getHeightMM();
	float depth_mm = m_gl_mgr->getDepthMM();

	// set initial location of axis center
	float axis_center_z;
	if (axial_loc < 0)
		axis_center_z = m_gl_mgr->getDepth() / 2;
	else
		axis_center_z = axial_loc;

	// init windowing
	int min_val = m_gl_mgr->getMinVal();
	int max_val = m_gl_mgr->getMaxVal();
	int default_window_center = m_gl_mgr->getWindowCenter();
	int default_window_width = m_gl_mgr->getWindowWidth();
	if (default_window_width == 0) {
		default_window_center = (min_val + max_val) / 2;
		default_window_width = max_val - min_val;
		if (default_window_width < 10)
			default_window_width = 10;
	}

	
	m_axis_center_forPath = cgip::CgipPoint(
		(m_gl_mgr->getWidth() / 2) * m_gl_mgr->getSpacingX(),
		(m_gl_mgr->getHeight() / 2) * m_gl_mgr->getSpacingY(),
		axis_center_z * m_gl_mgr->getSpacingZ());
	
	switch (slice_type) {
	case AXIAL_SLICE:
		m_axial_size = std::max(width_mm, height_mm) + SLICE_MARGIN;
		init_axial_size = m_axial_size;
		m_axial_w_sampling_len = m_axial_size / (m_w_sampling_cnt - 1);
		m_axial_h_sampling_len = m_axial_size / (m_h_sampling_cnt - 1);
		
		m_axial_n_dir = cgip::CgipVector(0, 0, 1);
		m_axial_w_dir = cgip::CgipVector(1, 0, 0);
		m_axial_h_dir = cgip::CgipVector(0, 1, 0);
		//m_angle_on_axial = 0;
		m_axial_thickness = 0;
		m_axial_window_level = default_window_center;
		m_axial_window_width = default_window_width;

		m_n_sampling_len = depth_mm / (m_w_sampling_cnt - 1);

		m_axis_center_forCurve = cgip::CgipPoint(
			(m_gl_mgr->getWidth() / 2) * m_gl_mgr->getSpacingX(),
			(m_gl_mgr->getHeight() / 2) * m_gl_mgr->getSpacingY(),
			0);

	
		m_axial_topleft_forCurve = m_axis_center_forCurve + (m_axial_w_dir + m_axial_h_dir) * (-m_axial_size / 2);
		m_axial_topleft_forPath = m_axis_center_forPath + (m_axial_w_dir + m_axial_h_dir) * (-m_axial_size / 2);

		break;
	case SAGITTAL_SLICE:
		m_sagittal_size = std::max(depth_mm, height_mm) + SLICE_MARGIN;
		init_sagittal_size = m_sagittal_size;
		m_sagittal_w_sampling_len = m_sagittal_size / (m_w_sampling_cnt - 1);
		m_sagittal_h_sampling_len = m_sagittal_size / (m_h_sampling_cnt - 1);
		m_sagittal_n_dir = cgip::CgipVector(1, 0, 0);
		m_sagittal_w_dir = cgip::CgipVector(0, 1, 0);
		m_sagittal_h_dir = cgip::CgipVector(0, 0, 1);
		//m_angle_on_sagittal = 0;
		m_sagittal_thickness = 0;
		m_sagittal_window_level = default_window_center;
		m_sagittal_window_width = default_window_width;
		
		m_n_sampling_len = width_mm / (m_w_sampling_cnt - 1);


		m_axis_center_forCurve = CgipPoint(0,
			(m_gl_mgr->getHeight() / 2) * m_gl_mgr->getSpacingY(),
			(m_gl_mgr->getDepth() / 2) * m_gl_mgr->getSpacingZ());


		m_sagittal_topleft_forCurve = m_axis_center_forCurve + (m_sagittal_w_dir + m_sagittal_h_dir) * (-m_sagittal_size / 2);
		m_sagittal_topleft_forPath = m_axis_center_forPath + (m_sagittal_w_dir + m_sagittal_h_dir) * (-m_sagittal_size / 2);


		break;
	case CORONAL_SLICE:
		m_coronal_size = std::max(width_mm, depth_mm) + SLICE_MARGIN;
		init_coronal_size = m_coronal_size;
		m_coronal_w_sampling_len = m_coronal_size / (m_w_sampling_cnt - 1);
		m_coronal_h_sampling_len = m_coronal_size / (m_h_sampling_cnt - 1);
		m_coronal_n_dir = cgip::CgipVector(0, 1, 0);
		m_coronal_w_dir = cgip::CgipVector(1, 0, 0);
		m_coronal_h_dir = cgip::CgipVector(0, 0, 1);
		//m_angle_on_coronal = 0;
		m_coronal_thickness = 0;
		m_coronal_window_level = default_window_center;
		m_coronal_window_width = default_window_width;
		m_n_sampling_len = height_mm / (m_w_sampling_cnt - 1);

		m_axis_center_forCurve = cgip::CgipPoint(
			(m_gl_mgr->getWidth() / 2) * m_gl_mgr->getSpacingX(),
			0,
			(m_gl_mgr->getDepth() / 2) * m_gl_mgr->getSpacingZ());

		m_coronal_topleft_forCurve = m_axis_center_forCurve + (m_coronal_w_dir + m_coronal_h_dir) * (-m_coronal_size / 2);
		m_coronal_topleft_forPath = m_axis_center_forPath + (m_coronal_w_dir + m_coronal_h_dir) * (-m_coronal_size / 2);

		break;
	}

	//_resetTopLeftPt();
}



CgipPoint CPRMgr::mapViewTo3DPointF(SliceType slice_type, float x, float y, bool for_curve) {

	CgipPoint pointInPlane;

	switch (slice_type) {
	case AXIAL_SLICE:
		if (for_curve) {
			pointInPlane = m_axial_topleft_forCurve + (x * m_axial_w_dir + y * m_axial_h_dir) * m_axial_size;
		}
		else {
			pointInPlane = m_axial_topleft_forPath + (x * m_axial_w_dir + y * m_axial_h_dir) * m_axial_size;
		}
		
		break;
	case CORONAL_SLICE:
		if (for_curve) {
			pointInPlane = m_coronal_topleft_forCurve + (x * m_coronal_w_dir + y * m_coronal_h_dir) * m_coronal_size;
		}
		else {
			pointInPlane = m_coronal_topleft_forPath + (x * m_coronal_w_dir + y * m_coronal_h_dir) * m_coronal_size;
		}
		break;
	case SAGITTAL_SLICE:
		if (for_curve) {
			pointInPlane = m_sagittal_topleft_forCurve + (x * m_sagittal_w_dir + y * m_sagittal_h_dir) * m_sagittal_size;
		}
		else {
			pointInPlane = m_sagittal_topleft_forPath + (x * m_sagittal_w_dir + y * m_sagittal_h_dir) * m_sagittal_size;
		}
		break;
	}

	return pointInPlane;

}

void CPRMgr::cpr_path(SliceType slice_type, std::vector<float> point_x, std::vector<float> point_y, float pathLength) {

	std::vector<CgipPoint> curve_pointList;

	if (point_x.size() != point_y.size()) {
		std::cout << "error: different number of points x and y" << std::endl;
		return;
	}

	if (point_x.size() < m_w_sampling_cnt) {
		std::cout << "error should be more than sampling count" << std::endl;
	}	
	else {
		CgipPoint startPoint = mapViewTo3DPointF(slice_type, point_x[0], point_y[0], 0);
		CgipPoint endPoint = mapViewTo3DPointF(slice_type, point_x[point_x.size() - 1], point_y[point_y.size() - 1], 0);
		CgipVector path = endPoint - startPoint;
		//m_path_length = sqrt(path.x() * path.x() + path.y() * path.y());
		//m_path_sampling_len = m_path_length / m_w_sampling_cnt;
		m_path_length = pathLength;
		std::cout << "path_length : " << m_path_length << std::endl;
		std::cout << "point_x size: " << point_x.size();
		float sampling_width = m_path_length / m_w_sampling_cnt;

		for (int i = 0; i < m_w_sampling_cnt; i++) {

			float sampled_index = i * sampling_width;
			int ceil_index = static_cast<int> (ceil(sampled_index));
			int floor_index = static_cast<int>(floor(sampled_index));
			float ratio = sampled_index - floor_index;

			float diff_x = point_x[ceil_index] - point_x[floor_index];
			float diff_y = point_y[ceil_index] - point_y[floor_index];

			//float ceil_floor_length = sqrt(diff_x * diff_x + diff_y * diff_y);

			float final_x = point_x[floor_index] + diff_x * ratio;
			float final_y = point_y[floor_index] + diff_y * ratio;

			CgipPoint curve_point = mapViewTo3DPointF(slice_type, point_x[i], point_y[i], 0);

			if (slice_type == AXIAL_SLICE) {
				curve_point = curve_point - m_axial_n_dir * (m_path_length * m_gl_mgr->getSpacingZ() / 2);
				m_path_sampling_len = m_path_length * m_gl_mgr->getSpacingZ() / m_w_sampling_cnt;
			}
			else if (slice_type == CORONAL_SLICE){
				curve_point = curve_point - m_coronal_n_dir * (m_path_length * m_gl_mgr->getSpacingY() / 2);
				m_path_sampling_len = m_path_length * m_gl_mgr->getSpacingY() / m_w_sampling_cnt;

			}
			else {
				curve_point = curve_point - m_sagittal_n_dir * (m_path_length * m_gl_mgr->getSpacingX() / 2);
				m_path_sampling_len = m_path_length * m_gl_mgr->getSpacingX() / m_w_sampling_cnt;

			}
			m_pathPoints[i].x = curve_point.x();
			m_pathPoints[i].y = curve_point.y();
			m_pathPoints[i].z = curve_point.z();
			
			//if (i == 0) {
			//	startPoint.setXYZ(curve_point.x(), curve_point.y(), curve_point.z());
			//}
			//
			//if (i == m_w_sampling_cnt - 1) {
			//	endPoint.setXYZ(curve_point.x(), curve_point.y(), curve_point.z());
			//}
		}

	}

	

	if (cpr_pathPoints) {
		SAFE_DELETE_OBJECT(cpr_pathPoints);
	}

	cpr_pathPoints = new Texture(m_pathPoints, m_w_sampling_cnt, 3);

}




void CPRMgr::cpr_curve(SliceType slice_type, std::vector<float> point_x, std::vector<float> point_y, int numOfPoints) {

	std::vector<CgipPoint> curve_pointList;

	std::cout << "point_x size: " << point_x.size() << std::endl;

	if (point_x.size() != point_y.size()) {
		std::cout << "error: different number of points x and y" << std::endl;
		return;
	}

	//if (point_x.size() < m_w_sampling_cnt) {
	//	std::cout << "error should be more than sampling count" << std::endl;
	//}
	//else {
	float sampling_width = (point_x.size() -1) / m_w_sampling_cnt;
	for (int i = 0; i < m_w_sampling_cnt; i++) {
		float sampled_index = i * sampling_width;
		int ceil_index = static_cast<int> (ceil(sampled_index));
		int floor_index = static_cast<int>(floor(sampled_index));
		float ratio = sampled_index - floor_index;
		
		float diff_x = point_x[ceil_index] - point_x[floor_index];
		float diff_y = point_y[ceil_index] - point_y[floor_index];
	
		//float ceil_floor_length = sqrt(diff_x * diff_x + diff_y * diff_y);
	
		float final_x = point_x[floor_index] + diff_x * ratio;
		float final_y = point_y[floor_index] + diff_y * ratio;
	
		//CgipPoint curve_point = mapViewTo3DPointF(slice_type, point_x[i], point_y[i], 1);
		CgipPoint curve_point = mapViewTo3DPointF(slice_type, final_x, final_y, 1);
	
		m_curvePoints[i].x = curve_point.x();
		m_curvePoints[i].y = curve_point.y();
		m_curvePoints[i].z = curve_point.z();
		//if (i == 0) {
		//	std::cout << "i == 0 widget x : " << point_x[i] << std::endl;
		//	std::cout << "i == 0 widget y : " << point_y[i] << std::endl;
		//	std::cout << "i == 0 mapViewTo3DPointF x : " << curve_point.x() << std::endl;
		//	std::cout << "i == 0 mapViewTo3DPointF y : " << curve_point.y() << std::endl;
		//}
		//
		//if (i == m_w_sampling_cnt - 1) {
		//	std::cout << "i == 1023 widget x : " << point_x[i] << std::endl;
		//	std::cout << "i == 1023 widget y : " << point_y[i] << std::endl;
		//	std::cout << "i == 1023 mapViewTo3DPointF x : " << curve_point.x() << std::endl;
		//	std::cout << "i == 1023 mapViewTo3DPointF y : " << curve_point.y() << std::endl;
		//}
	}

	//}

	if (cpr_curvePoints) {
		SAFE_DELETE_OBJECT(cpr_curvePoints);
	}

	cpr_curvePoints = new Texture(m_curvePoints, m_w_sampling_cnt, 2);
	
}

void CPRMgr::setCPRPath(SliceType slice_type) {

	int cnt = 0;
	float dist = 0;

	switch (slice_type) {
	case AXIAL_SLICE: {
		bool is_thickness = !(int(m_axial_thickness / 2) < 1);
		if (is_thickness) {
			cnt = int(m_axial_thickness / 2);
			dist = _get_dist_next_slice(1, m_axis_center_forPath.z(), m_gl_mgr->getSpacingZ());
		}
		*m_cpr_path = std::move(_setCPRPath(0, m_axial_topleft_forPath, m_indicator_dir, m_axial_n_dir, m_w_sampling_cnt,
			m_h_sampling_cnt, cnt, m_path_sampling_len, m_path_sampling_len,
			is_thickness, m_axial_window_width, m_axial_window_level, m_axial_inverse));
		break;
	}
	case CORONAL_SLICE: {
		bool is_thickness = !(int(m_coronal_thickness / 2) < 1);
		if (is_thickness) {
			cnt = int(m_coronal_thickness / 2);
			dist = _get_dist_next_slice(1, m_axis_center_forPath.z(), m_gl_mgr->getSpacingZ());
		}
		*m_cpr_path = std::move(_setCPRPath(1, m_coronal_topleft_forPath, m_indicator_dir, m_coronal_n_dir, m_w_sampling_cnt,
			m_h_sampling_cnt, cnt, m_path_sampling_len, m_path_sampling_len,
			is_thickness, m_coronal_window_width, m_coronal_window_level, m_coronal_inverse));
		break;
	}
	case SAGITTAL_SLICE: {
		bool is_thickness = !(int(m_sagittal_thickness / 2) < 1);
		if (is_thickness) {
			cnt = int(m_sagittal_thickness / 2);
			dist = _get_dist_next_slice(1, m_axis_center_forPath.z(), m_gl_mgr->getSpacingZ());
		}

		*m_cpr_path = std::move(_setCPRPath(1, m_sagittal_topleft_forPath, m_indicator_dir, m_sagittal_n_dir, m_w_sampling_cnt,
			m_h_sampling_cnt, cnt, m_path_sampling_len, m_path_sampling_len,
			is_thickness, m_sagittal_window_width, m_sagittal_window_level, m_sagittal_inverse));
		break;
	}
	}
}




CgipPlane CPRMgr::_setCPRPath(int slice_type_num,
	CgipPoint& startingPoint,
	CgipVector indicator_dir,
	CgipVector n_dir,
	int w_cnt,
	int h_cnt,
	int n_cnt,
	float path_sampling_len,
	float n_sampling_len,
	bool is_thickness,
	int window_width,
	int window_level,
	bool is_inverse) {

	glfwMakeContextCurrent(m_gl_mgr->getGLWindow());
	m_gl_mgr->setVert(m_w_sampling_cnt, m_h_sampling_cnt);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 m_camera = glm::translate(glm::mat4(1), glm::vec3(-(w_cnt) / 2, -(w_cnt) / 2, 0.01 / 2));
	glm::mat4 m_proj = glm::scale(glm::mat4(1), glm::vec3(2 / (float)(w_cnt), 2 / (float)w_cnt, 1 / (10000.0f - 0.01f)));

	m_gl_mgr->getCPRPathShader()->use();

	m_gl_mgr->getCPRPathShader()->setMat4("mvMatrix", m_camera);
	m_gl_mgr->getCPRPathShader()->setMat4("projMatrix", m_proj);

	m_gl_mgr->getCPRPathShader()->setVec3("startingPoint", glm::vec3(startingPoint.x(), startingPoint.y(), startingPoint.z()));
	m_gl_mgr->getCPRPathShader()->setVec3("indicator_dir", glm::vec3(indicator_dir.x(), indicator_dir.y(), indicator_dir.z()));
	m_gl_mgr->getCPRPathShader()->setVec3("n_dir", glm::vec3(n_dir.x(), n_dir.y(), n_dir.z()));
	

	m_gl_mgr->getCPRPathShader()->setFloat("m_spacingX", m_gl_mgr->getSpacingX());
	m_gl_mgr->getCPRPathShader()->setFloat("m_spacingY", m_gl_mgr->getSpacingY());
	m_gl_mgr->getCPRPathShader()->setFloat("m_spacingZ", m_gl_mgr->getSpacingZ());

	m_gl_mgr->getCPRPathShader()->setInt("N_x", m_gl_mgr->getWidth());
	m_gl_mgr->getCPRPathShader()->setInt("N_y", m_gl_mgr->getHeight());
	m_gl_mgr->getCPRPathShader()->setInt("N_z", m_gl_mgr->getDepth());

	m_gl_mgr->getCPRPathShader()->setInt("w_cnt", w_cnt);
	m_gl_mgr->getCPRPathShader()->setInt("h_cnt", h_cnt);
	m_gl_mgr->getCPRPathShader()->setInt("n_cnt", n_cnt);

	m_gl_mgr->getCPRPathShader()->setFloat("path_len", path_sampling_len);
	m_gl_mgr->getCPRPathShader()->setFloat("n_len", n_sampling_len);

	//std::cout << "path_len: " << path_sampling_len << std::endl;
	//std::cout << "npath_dir: " << indicator_dir.x() << ", " << indicator_dir.y() << ", " << indicator_dir.z() << std::endl;
	//
	//std::cout << "n_len: " << n_sampling_len << std::endl;
	//std::cout << "n_dir: " << n_dir.x() << ", " << n_dir.y() << ", " << n_dir.z() << std::endl;

	m_gl_mgr->getCPRPathShader()->setFloat("window_level", window_level - m_gl_mgr->getMinVal());
	m_gl_mgr->getCPRPathShader()->setFloat("window_width", window_width);

	m_gl_mgr->getCPRPathShader()->setBool("thickness", is_thickness);
	m_gl_mgr->getCPRPathShader()->setInt("thickness_mode", m_thickness_mode);
	m_gl_mgr->getCPRPathShader()->setBool("inverse", is_inverse);
	m_gl_mgr->getCPRPathShader()->setInt("slice_type", slice_type_num);

	m_gl_mgr->setMaskShaderParam(MPR);

	m_gl_mgr->getCPRPathShader()->setInt("volume_data", 0);
	m_gl_mgr->getCPRPathShader()->setInt("mask_data", 1);
	m_gl_mgr->getCPRPathShader()->setInt("rayStartingPoint", 2);
	m_gl_mgr->getCPRPathShader()->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_gl_mgr->getDataTextureId());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_gl_mgr->getMaskTextureId());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_1D, cpr_pathPoints->ID);


	glBindVertexArray(m_gl_mgr->getVAOId());

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, w_cnt, h_cnt);
	glDrawArrays(GL_TRIANGLES, 0, 6 * w_cnt * h_cnt);

	glBindTexture(GL_TEXTURE_2D, 0);

	GLubyte* imageData = SAFE_ALLOC_2D(GLubyte, w_cnt * h_cnt, 3);

	glFinish();
	glReadPixels(0, 0, w_cnt, h_cnt, GL_RGB, GL_UNSIGNED_BYTE, imageData);

	cgip::CgipPlane new_plane(w_cnt, h_cnt, 3);

	for (unsigned int i = 0; i < new_plane.getWidth(); ++i)
	{
		for (unsigned int j = 0; j < new_plane.getHeight(); ++j)
		{
			new_plane.setPixelValue(j, i, 0, imageData[3 * w_cnt * i + 3 * j + 0]);
			new_plane.setPixelValue(j, i, 1, imageData[3 * w_cnt * i + 3 * j + 1]);
			new_plane.setPixelValue(j, i, 2, imageData[3 * w_cnt * i + 3 * j + 2]);
		}
	}

	glfwSwapBuffers(m_gl_mgr->getGLWindow());

	SAFE_DELETE_ARRAY(imageData);

	return new_plane;
}



void CPRMgr::setCPR(SliceType slice_type) {

	int cnt = 0;
	float dist = 0;

	switch (slice_type) {
	case AXIAL_SLICE: {
		bool is_thickness = !(int(m_axial_thickness / 2) < 1);
		if (is_thickness) {
			cnt = int(m_axial_thickness / 2);
			dist = _get_dist_next_slice(1, m_axis_center_forCurve.z(), m_gl_mgr->getSpacingZ());
		}
		*m_cpr = std::move(_setCPR(0, m_axial_topleft_forCurve, m_axial_w_dir, m_axial_h_dir, m_axial_n_dir, m_w_sampling_cnt,
			m_h_sampling_cnt, cnt, m_axial_w_sampling_len, m_axial_h_sampling_len, m_n_sampling_len,
			is_thickness, m_axial_window_width, m_axial_window_level, m_axial_inverse));
		break;
	}
	case CORONAL_SLICE: {
		bool is_thickness = !(int(m_coronal_thickness / 2) < 1);
		if (is_thickness) {
			cnt = int(m_coronal_thickness / 2);
			dist = _get_dist_next_slice(1, m_axis_center_forCurve.z(), m_gl_mgr->getSpacingZ());
		}
		*m_cpr = std::move(_setCPR(1, m_coronal_topleft_forCurve, m_coronal_w_dir, m_coronal_h_dir, m_coronal_n_dir, m_w_sampling_cnt,
			m_h_sampling_cnt, cnt, m_coronal_w_sampling_len, m_coronal_h_sampling_len, m_n_sampling_len,
			is_thickness, m_coronal_window_width, m_coronal_window_level, m_coronal_inverse));
		break;
	}
	case SAGITTAL_SLICE: {
		bool is_thickness = !(int(m_sagittal_thickness / 2) < 1);
		if (is_thickness) {
			cnt = int(m_sagittal_thickness / 2);
			dist = _get_dist_next_slice(1, m_axis_center_forCurve.z(), m_gl_mgr->getSpacingZ());
		}
		*m_cpr = std::move(_setCPR(1, m_sagittal_topleft_forCurve, m_sagittal_w_dir, m_sagittal_h_dir, m_sagittal_n_dir, m_w_sampling_cnt,
			m_h_sampling_cnt, cnt, m_sagittal_w_sampling_len, m_sagittal_h_sampling_len, m_n_sampling_len,
			is_thickness, m_sagittal_window_width, m_sagittal_window_level, m_sagittal_inverse));
		break;
	}
	}
}




CgipPlane CPRMgr::_setCPR(int slice_type_num,
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
	bool is_thickness,
	int window_width,
	int window_level,
	bool is_inverse) {

		glfwMakeContextCurrent(m_gl_mgr->getGLWindow());
		m_gl_mgr->setVert(m_w_sampling_cnt, m_h_sampling_cnt);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 m_camera = glm::translate(glm::mat4(1), glm::vec3(-(w_cnt) / 2, -(w_cnt) / 2, 0.01 / 2));
		glm::mat4 m_proj = glm::scale(glm::mat4(1), glm::vec3(2 / (float)(w_cnt), 2 / (float)w_cnt, 1 / (10000.0f - 0.01f)));

		m_gl_mgr->getCPRShader()->use();

		m_gl_mgr->getCPRShader()->setMat4("mvMatrix", m_camera);
		m_gl_mgr->getCPRShader()->setMat4("projMatrix", m_proj);

		m_gl_mgr->getCPRShader()->setVec3("P_screen", glm::vec3(topleft_p.x(), topleft_p.y(), topleft_p.z()));
		m_gl_mgr->getCPRShader()->setVec3("w_dir", glm::vec3(w_dir.x(), w_dir.y(), w_dir.z()));
		m_gl_mgr->getCPRShader()->setVec3("h_dir", glm::vec3(h_dir.x(), h_dir.y(), h_dir.z()));
		m_gl_mgr->getCPRShader()->setVec3("n_dir", glm::vec3(n_dir.x(), n_dir.y(), n_dir.z()));

		m_gl_mgr->getCPRShader()->setFloat("m_spacingX", m_gl_mgr->getSpacingX());
		m_gl_mgr->getCPRShader()->setFloat("m_spacingY", m_gl_mgr->getSpacingY());
		m_gl_mgr->getCPRShader()->setFloat("m_spacingZ", m_gl_mgr->getSpacingZ());

		m_gl_mgr->getCPRShader()->setInt("N_x", m_gl_mgr->getWidth());
		m_gl_mgr->getCPRShader()->setInt("N_y", m_gl_mgr->getHeight());
		m_gl_mgr->getCPRShader()->setInt("N_z", m_gl_mgr->getDepth());

		m_gl_mgr->getCPRShader()->setInt("w_cnt", w_cnt);
		m_gl_mgr->getCPRShader()->setInt("h_cnt", h_cnt);
		m_gl_mgr->getCPRShader()->setInt("n_cnt", n_cnt);

		m_gl_mgr->getCPRShader()->setFloat("w_len", w_len);
		m_gl_mgr->getCPRShader()->setFloat("h_len", h_len);
		m_gl_mgr->getCPRShader()->setFloat("n_len", n_len);


		m_gl_mgr->getCPRShader()->setFloat("window_level", window_level - m_gl_mgr->getMinVal());
		m_gl_mgr->getCPRShader()->setFloat("window_width", window_width);

		m_gl_mgr->getCPRShader()->setBool("thickness", is_thickness);
		m_gl_mgr->getCPRShader()->setInt("thickness_mode", m_thickness_mode);
		m_gl_mgr->getCPRShader()->setBool("inverse", is_inverse);
		m_gl_mgr->getCPRShader()->setInt("slice_type", slice_type_num);


		m_gl_mgr->setMaskShaderParam(MPR);

		m_gl_mgr->getCPRShader()->setInt("volume_data", 0);
		m_gl_mgr->getCPRShader()->setInt("mask_data", 1);
		m_gl_mgr->getCPRShader()->setInt("rayStartingPoint", 2);

		m_gl_mgr->getCPRShader()->use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, m_gl_mgr->getDataTextureId());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, m_gl_mgr->getMaskTextureId());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, cpr_curvePoints->ID);

		glBindVertexArray(m_gl_mgr->getVAOId());

		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, w_cnt, h_cnt);
		glDrawArrays(GL_TRIANGLES, 0, 6 * w_cnt * h_cnt);

		glBindTexture(GL_TEXTURE_2D, 0);

		GLubyte* imageData = SAFE_ALLOC_2D(GLubyte, w_cnt * h_cnt, 3);

		glFinish();
		glReadPixels(0, 0, w_cnt, h_cnt, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		cgip::CgipPlane new_plane(w_cnt, h_cnt, 3);

		for (unsigned int i = 0; i < new_plane.getWidth(); ++i)
		{
			for (unsigned int j = 0; j < new_plane.getHeight(); ++j)
			{
				new_plane.setPixelValue(j, i, 0, imageData[3 * w_cnt * i + 3 * j + 0]);
				new_plane.setPixelValue(j, i, 1, imageData[3 * w_cnt * i + 3 * j + 1]);
				new_plane.setPixelValue(j, i, 2, imageData[3 * w_cnt * i + 3 * j + 2]);
			}
		}

		glfwSwapBuffers(m_gl_mgr->getGLWindow());

		SAFE_DELETE_ARRAY(imageData);

		return new_plane;
}



bool CPRMgr::is_outsideVolume(cgip::CgipPoint p)
{
	return (p.x() < 0 || m_vol_mgr->getWidthMM() < p.x() ||
		p.y() < 0 || m_vol_mgr->getHeightMM() < p.y() ||
		p.z() < 0 || m_vol_mgr->getDepthMM() < p.z());
}

float CPRMgr::_get_projection(cgip::CgipVector a, cgip::CgipVector b)
{
	float result, pr;
	pr = a.dot(b);
	result = (pr * b).len();
	if (pr < 0)
		result *= -1;

	return result;
}


float CPRMgr::_get_dist_next_slice(float d, float cur_loc, float spacing) {
	float temp_loc, next_loc;
	cgip::CgipPoint new_axis_center;

	temp_loc = cur_loc / spacing;
	next_loc = floor(temp_loc);
	if (temp_loc - next_loc < EPSILON)
		next_loc += d;
	else if (1 + next_loc - temp_loc < EPSILON)
		next_loc += d + 1;
	else
		next_loc += 1;

	next_loc *= spacing;
	d = next_loc - cur_loc;

	return d;
}