#include "DVRRectMgr.h"

DVRRectMgr::DVRRectMgr() {
	m_dvr = new cgip::CgipPlane;
	sculpt_plane = new cgip::CgipPlane;
	arcball = new ArcBall();
}

DVRRectMgr::~DVRRectMgr() {
	SAFE_DELETE_OBJECT(m_dvr);
	SAFE_DELETE_OBJECT(m_tf);
	SAFE_DELETE_OBJECT(tex_tf);
	SAFE_DELETE_OBJECT(sculpt_texture);
	SAFE_DELETE_OBJECT(sculpt_plane);
	SAFE_DELETE_OBJECT(arcball);
	SAFE_DELETE_OBJECT(m_tex_sculpt);
}

void DVRRectMgr::init_DVR(GLMgr* gl_mgr, VolumeMgr* vol_mgr) {

	m_gl_mgr = gl_mgr;
	m_volume_mgr = vol_mgr;


	m_min_val = -2048;
	m_max_val = 4000;

	m_unit_ray_len = 1;

	m_width = m_gl_mgr->getWidth();
	m_height = m_gl_mgr->getHeight();
	m_depth = m_gl_mgr->getDepth();


	m_spacingX = m_gl_mgr->getSpacingX();
	m_spacingY = m_gl_mgr->getSpacingY();
	m_spacingZ = m_gl_mgr->getSpacingZ();

	m_tf = new TF(m_max_val - m_min_val, m_min_val);
	std::memset(m_tf->getTF(), 0, sizeof(TF_BGRA) * (m_max_val - m_min_val));
	dvr_histogram = SAFE_ALLOC_1D(int, m_max_val - m_min_val);
	std::memset(dvr_histogram, 0, sizeof(int) * (m_max_val - m_min_val));
	//volume_data = m_volume_mgr->getRawPtr();
	int tf_size = m_max_val - m_min_val;
	ofstream fout;
	if (volume_data) {
		fout.open("histogram.txt");
		for (CgipInt ijk = 0; ijk < m_height * m_width * m_depth; ijk++) {
			CgipInt i = ijk / (m_width * m_height);
			CgipInt j = (ijk % (m_width * m_height)) / m_width;
			CgipInt k = (ijk % (m_width * m_height)) % m_width;

			short temp = m_volume_mgr->getRawPtr()->getRawPtr()[i][j * m_width + k];
			if (m_min_val < 0) {
				temp -= m_min_val;		 // move to 0~4095
			}
			else {
				temp += m_min_val;
			}

			if (temp > tf_size - 1) {
				dvr_histogram[tf_size - 1]++;
			}
			else if (temp < 0) {
				dvr_histogram[0]++;
			}
			else {
				dvr_histogram[temp]++;
			}
		}

		for (int l = 0; l < tf_size; l++) {
			fout << l + m_min_val << '\t' << dvr_histogram[l] << std::endl;
		}
		fout.close();

	}
	printf("histogram complete\n");




	// Set Data
	m_num_max = std::max((int)(m_width * m_spacingX), std::max((int)(m_height * m_spacingY), (int)(m_depth * m_spacingZ)));
	float m_higher_image = std::max(m_width, m_height);
	int m_higher_panel = std::max(m_panel_height, m_panel_width);
	m_screen_dist = m_num_max * 5;

	m_tex_sculpt = SAFE_ALLOC_2D(unsigned char, m_width * m_height, m_depth);

	arcball->setWidthHeight(m_panel_width, m_panel_height);
	arcball->setPrincipalAxis(m_volume_mgr->getPAxis());

	_setSculptTextureData();

	glfwMakeContextCurrent(m_gl_mgr->getGLWindow());
	if (tex_tf)
		SAFE_DELETE_OBJECT(tex_tf);

	tex_tf = new Texture(m_tf->getTF(), (m_max_val - m_min_val), 2);
	glfwSwapBuffers(m_gl_mgr->getGLWindow());
}

void DVRRectMgr::init_position() {
	m_translate_center = glm::translate(glm::mat4(1), glm::vec3(-m_width * m_spacingX / 2, -m_height * m_spacingY / 2, -m_depth * m_spacingZ / 2));
	m_translate_center_inverse = glm::translate(glm::mat4(1), glm::vec3(m_width * m_spacingX / 2, m_height * m_spacingY / 2, m_depth * m_spacingZ / 2));

	init_geometry();
}

void DVRRectMgr::init_geometry(VROrientationType orientation_type) {
	m_current_num_max = m_num_max;
	m_pixel_screen_len.x = m_current_num_max * m_panel_wh_ratio / (m_sampling_cnt.x - 1);

	m_pixel_screen_len.y = m_current_num_max / (m_sampling_cnt.y - 1);
	m_center_coordinate[0] = m_sampling_cnt.x / 2;
	m_center_coordinate[1] = m_sampling_cnt.y / 2;
	m_current_orientation_type = orientation_type;
	m_upsample_screen_diff = m_render_resolution - m_screen_width;

	switch (orientation_type) {
	case A:
		m_current_num_max = std::max((int)(m_width * m_spacingX), (int)(m_depth * m_spacingZ));

		P_screen = glm::vec3(m_width * m_spacingX / 2, m_height * m_spacingY / 2, m_depth * m_spacingZ / 2) + glm::vec3(-m_current_num_max * m_panel_wh_ratio / 2, -m_screen_dist, -m_current_num_max / 2);
		ws = glm::vec3(1, 0, 0);
		hs = glm::vec3(0, 0, 1);
		ns = glm::vec3(0, 1, 0);
		break;

	case P:
		m_current_num_max = std::max((int)(m_width * m_spacingX), (int)(m_depth * m_spacingZ));
		P_screen = glm::vec3(m_width * m_spacingX / 2, m_height * m_spacingY / 2, m_depth * m_spacingZ / 2) + glm::vec3((m_current_num_max + ((m_render_resolution - m_screen_width) / 2)) / 2, m_screen_dist, -m_current_num_max / 2);
		ws = glm::vec3(-1, 0, 0);
		hs = glm::vec3(0, 0, 1);
		ns = glm::vec3(0, -1, 0);
		break;

	case R:
		m_current_num_max = std::max((int)(m_height * m_spacingY), (int)(m_depth * m_spacingZ));
		P_screen = glm::vec3(m_width * m_spacingX / 2, m_height * m_spacingY / 2, m_depth * m_spacingZ / 2) + glm::vec3(m_screen_dist, m_current_num_max / 2, -m_current_num_max / 2);
		ws = glm::vec3(0, -1, 0);
		hs = glm::vec3(0, 0, 1);
		ns = glm::vec3(1, 0, 0);
		break;

	case L:
		m_current_num_max = std::max((int)(m_height * m_spacingY), (int)(m_depth * m_spacingZ));
		P_screen = glm::vec3(m_width * m_spacingX / 2, m_height * m_spacingY / 2, m_depth * m_spacingZ / 2) + glm::vec3(m_screen_dist, -m_current_num_max / 2, -m_current_num_max / 2);
		ws = glm::vec3(0, 1, 0);
		hs = glm::vec3(0, 0, 1);
		ns = glm::vec3(-1, 0, 0);
		break;

	case H:
		m_current_num_max = std::max((int)(m_height * m_spacingY), (int)(m_width * m_spacingX));
		P_screen = glm::vec3(m_width * m_spacingX / 2, m_height * m_spacingY / 2, m_depth * m_spacingZ / 2) + glm::vec3(m_current_num_max / 2, -m_current_num_max / 2, -m_screen_dist);
		ws = glm::vec3(-1, 0, 0);
		hs = glm::vec3(0, 1, 0);
		ns = glm::vec3(0, 0, 1);
		break;

	case F:
		m_current_num_max = std::max((int)(m_height * m_spacingY), (int)(m_width * m_spacingX));
		P_screen = glm::vec3(m_width * m_spacingX / 2, m_height * m_spacingY / 2, m_depth * m_spacingZ / 2) + glm::vec3(-m_current_num_max / 2, -m_current_num_max / 2, m_screen_dist);
		ws = glm::vec3(1, 0, 0);
		hs = glm::vec3(0, 1, 0);
		ns = glm::vec3(0, 0, -1);
		break;

	}


	init_current_num_max = m_current_num_max;
	init_PScreen = P_screen;
	init_pixel_screen_x = m_pixel_screen_len.x;


	trans = m_volume_mgr->getInitRotationMat();
	shear_mat = m_volume_mgr->getInitShearMat();
	std::cout << "DVR getting shear mat: " << glm::to_string(shear_mat) << std::endl;
	P_screen = glm::vec3(m_translate_center_inverse * trans * m_translate_center * glm::vec4(P_screen, 1));

	ws = glm::vec3(trans * glm::vec4(ws, 1));
	hs = glm::vec3(trans * glm::vec4(hs, 1));
	ns = glm::vec3(trans * glm::vec4(ns, 1));


	//zoom_by_ratio(0.7);
}

void DVRRectMgr::setDVR() {
	glfwMakeContextCurrent(m_gl_mgr->getGLWindow());

	m_gl_mgr->setVert(m_render_resolution, m_render_resolution);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_setMaterials();
	m_gl_mgr->getVRShader()->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_gl_mgr->getDataTextureId());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_gl_mgr->getMaskTextureId());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_1D, tex_tf->ID);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, sculpt_texture->ID);

	glBindVertexArray(m_gl_mgr->getVAOId());

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_screen_width, m_screen_height);

	glDrawArrays(GL_TRIANGLES, 0, 6 * m_sampling_cnt.x * m_sampling_cnt.y);

	glBindTexture(GL_TEXTURE_2D, 0);

	int bytesPerPixel = 3;
	int strideSize = (m_screen_width * (4 - bytesPerPixel)) % 4;
	int bytesPerLine = m_screen_width * 3 + strideSize;
	int size = bytesPerLine * m_screen_height;
	GLubyte* imageData = new GLubyte[size];

	glFinish();
	glReadPixels(0, 0, m_screen_width, m_screen_height, GL_RGB, GL_UNSIGNED_BYTE, imageData);

	//for debugging
	//QImage image(imageData, m_screen_width, m_screen_height, bytesPerLine, QImage::Format::Format_RGB888);
	//bool result = image.save("test_dvr_image.png", "PNG");


	glfwSwapBuffers(m_gl_mgr->getGLWindow());

	cgip::CgipPlane new_plane(m_screen_width, m_screen_height, 3);


	for (unsigned int i = 0; i < new_plane.getWidth(); ++i)
	{
		for (unsigned int j = 0; j < new_plane.getHeight(); ++j)
		{
			new_plane.setPixelValue(i, j, 0, imageData[bytesPerLine * j + 3 * i + 0]);
			new_plane.setPixelValue(i, j, 1, imageData[bytesPerLine * j + 3 * i + 1]);
			new_plane.setPixelValue(i, j, 2, imageData[bytesPerLine * j + 3 * i + 2]);
		}
	}


	glfwSwapBuffers(m_gl_mgr->getGLWindow());

	SAFE_DELETE_ARRAY(imageData);

	*m_dvr = std::move(new_plane);
}
void DVRRectMgr::zoom_by_ratio(float delta) {
	printf("delta: %f", delta);
	int d = -(m_current_num_max - (1 / delta * (init_current_num_max / m_current_num_max)) * m_current_num_max);
	//d *= -1;


	if (m_current_num_max + d < m_num_max / 10 || 4 * m_num_max < m_current_num_max + d)
		return;

	P_screen -= (ws * m_panel_wh_ratio + hs) * d / 2;
	m_center_coordinate = glm::vec2((m_center_coordinate.x * m_current_num_max / m_sampling_cnt.x + d / 2) * m_sampling_cnt.x / (m_current_num_max + d),
		(m_center_coordinate.y * m_current_num_max / m_sampling_cnt.y + d / 2) * m_sampling_cnt.y / (m_current_num_max + d));
	m_current_num_max += d;
	m_pixel_screen_len.x = (m_current_num_max * m_panel_wh_ratio) / (m_sampling_cnt.x - 1);

	m_pixel_screen_len.y = m_current_num_max / (m_sampling_cnt.y - 1);

}

void DVRRectMgr::zooming(int delta) {
	int d = 24;

	if (delta > 0)
		d *= -1;

	if (m_current_num_max + d < m_num_max / 10 || 4 * m_num_max < m_current_num_max + d)
		return;

	P_screen -= (ws * m_panel_wh_ratio + hs) * d / 2;
	m_center_coordinate = glm::vec2((m_center_coordinate.x * m_current_num_max / m_sampling_cnt.x + d / 2) * m_sampling_cnt.x / (m_current_num_max + d),
		(m_center_coordinate.y * m_current_num_max / m_sampling_cnt.y + d / 2) * m_sampling_cnt.y / (m_current_num_max + d));
	m_current_num_max += d;
	m_pixel_screen_len.x = (m_current_num_max * m_panel_wh_ratio) / (m_sampling_cnt.x - 1);
	m_pixel_screen_len.y = m_current_num_max / (m_sampling_cnt.y - 1);
}

void DVRRectMgr::panning(float dx, float dy) {
	glm::vec2 delta(dx, dy);
	m_center_coordinate += delta;

	dx = dx * m_pixel_screen_len.x * m_sampling_cnt.x / m_sampling_cnt.x;
	dy = dy * m_pixel_screen_len.y * m_sampling_cnt.y / m_sampling_cnt.y;
	P_screen = P_screen - dx * ws + dy * hs;
}

void DVRRectMgr::rotation(int x, int y, int width, int height) {

	int smallerAxisSize = std::min(width, height);

	if (rotate_start_height != height || rotate_start_width != width) {
		arcball->setWidthHeight(width, height);
	}

	int startX = m_rotate_start_x - (width / 2 - smallerAxisSize / 2);
	int endX = x - (width / 2 - smallerAxisSize / 2);

	int startY = m_rotate_start_y - (height / 2 - smallerAxisSize / 2);
	int endY = y - (height / 2 - smallerAxisSize / 2);




	glm::vec3 startPos = arcball->getArcballVector(startX, startY);
	glm::vec3 endPos = arcball->getArcballVector(endX, endY);


	if (startPos == endPos) return;


	float rotation_angle = std::acos(std::min(1.0f, glm::dot(startPos, endPos))) * 180.0 / PI;

	glm::vec3 cross = glm::cross(startPos, endPos);
	if (m_volume_mgr->getPAxis() == 1) {
		cross = cross * -1;
	}
	glm::vec3 rotation_axis = cross.x * ws - cross.y * ns + cross.z * hs;
	glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation_angle) * 3, rotation_axis);

	trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(rotation_angle), rotation_axis);


	P_screen = glm::vec3(m_translate_center_inverse * trans * m_translate_center * glm::vec4(P_screen, 1));



	ws = glm::vec3(trans * glm::vec4(ws, 1));
	hs = glm::vec3(trans * glm::vec4(hs, 1));
	ns = glm::vec3(trans * glm::vec4(ns, 1));



	m_rotate_start_x = x;
	m_rotate_start_y = y;
	rotate_start_width = width;
	rotate_start_height = height;

}

void DVRRectMgr::doSculpt(std::vector<int> p_x, std::vector<int> p_y) {
	m_is_sculpt = true;
	m_is_voi = false;
	*sculpt_plane = std::move(cgip::CgipPlane(m_render_resolution, m_render_resolution));
	std::vector<cgip::CgipPoint> points;

	for (int i = 0; i < p_x.size(); i++) {
		points.push_back(cgip::CgipPoint(p_x[i] * m_render_resolution / m_panel_width,
			p_y[i] * m_render_resolution / m_panel_height, 0));
	}

	_HoleFilling(points, 1, sculpt_plane);

	_setSculptTextureData();
}

void DVRRectMgr::doVOI(std::vector<int> p_x, std::vector<int> p_y) {
	m_is_voi = true;
	m_is_sculpt = false;

	*sculpt_plane = std::move(cgip::CgipPlane(m_render_resolution, m_render_resolution));
	std::vector<cgip::CgipPoint> points;

	for (int i = 0; i < p_x.size(); i++) {
		points.push_back(cgip::CgipPoint(p_x[i] * m_render_resolution / m_panel_width,
			p_y[i] * m_render_resolution / m_panel_height, 0));
	}

	_HoleFilling(points, 1, sculpt_plane);

	_setSculptTextureData();
}

void DVRRectMgr::resetSculpt() {
	m_is_sculpt = false;
	m_is_voi = false;

	_setSculptTextureData();
}

void DVRRectMgr::changeProjection(int type, std::vector<std::vector<std::vector<int>>> coords) {
	m_projection_type = type;

	/*
	type 0 ==> OTF
	type 1 ==> MaxIP
	type 2 ==> MinIP
	type 3 ==> Raysum
	*/
	if (type == 0) {

		int minPos = m_max_val - m_min_val;
		int TF_width = m_max_val - m_min_val;
		float dynamicRange = TF_HEIGHT - 2 * TF_MARGIN;
		float inc = 0.0f;
		for (int i = 0; i < TF_width; i++) {
			m_tf->getTF()[i].a = 0.0f;
		}

		for (int j = 0; j < coords.size(); j++) {
			std::vector<std::vector<int>> ptList = coords[j];		//PointList for the polygon

			if (ptList[0][0] < minPos) {
				minPos = ptList[0][0];
			}
			for (int k = 0; k < ptList.size() - 1; k++) {
				int start_intensity = ptList[k][0];
				int end_intensity = ptList[k + 1][0];


				int startIdx = start_intensity;
				int endIdx = end_intensity;

				float inc = (ptList[k][1] - ptList[k + 1][1]) / static_cast<float>(end_intensity - start_intensity);    //slope
				float yPos = TF_HEIGHT - ptList[k][1] - TF_MARGIN;

				for (int l = startIdx; l < endIdx; l++) {
					if (l < 0 || l >= TF_width)    continue;
					float val = yPos / dynamicRange;
					if (m_tf->getTF()[l].a < val) {
						m_tf->getTF()[l].a = val;
					}
					yPos += inc;

				}

			}
		}
		//Pre-Integrated Classification (1D - Approximated ver.)
		if (minPos <= 0)    minPos = 1;
		for (int i = minPos; i < TF_width; i++)
			m_tf->getTF()[i].a += m_tf->getTF()[i - 1].a;

		glfwMakeContextCurrent(m_gl_mgr->getGLWindow());
		if (tex_tf)
			SAFE_DELETE_OBJECT(tex_tf);

		tex_tf = new Texture(m_tf->getTF(), (m_max_val - m_min_val), 2);
		glfwSwapBuffers(m_gl_mgr->getGLWindow());


	}
}

void DVRRectMgr::changeColor(std::map<int, std::vector<int>> colorMap) {

	int cur_intensity = 0;
	int prev_intensity = 0;
	int last_intensity = 0;
	std::vector<int> prev_color = { 255, 255, 255 };
	std::vector<int> last_color = { 255, 255, 255 };
	int i = 0;

	int tf_width = m_max_val - m_min_val;

	std::map<int, std::vector<int>>::iterator it;
	for (it = colorMap.begin(); it != colorMap.end(); it++) {
		cur_intensity = it->first;
		std::vector<int> cur_color = it->second;


		if (i == 0) {
			int nMin = std::min(cur_intensity, (m_max_val - m_min_val));
			last_intensity = nMin;

			for (int j = 0; j < nMin; j++) {
				m_tf->getTF()[j].r = cur_color[0] / 255.0;
				m_tf->getTF()[j].g = cur_color[1] / 255.0;
				m_tf->getTF()[j].b = cur_color[2] / 255.0;
			}
			prev_intensity = cur_intensity;
			prev_color = cur_color;
			last_color = cur_color;
			i++;

		}
		else {

			int length = cur_intensity - prev_intensity;

			float dR = (cur_color[0] - prev_color[0]) / static_cast<float>(length);
			float dG = (cur_color[1] - prev_color[1]) / static_cast<float>(length);
			float dB = (cur_color[2] - prev_color[2]) / static_cast<float>(length);
			float valR = dR;
			float valG = dG;
			float valB = dB;

			int nMin = std::min(prev_intensity, (m_max_val - m_min_val));
			int nMax = std::min(cur_intensity, (m_max_val - m_min_val));
			last_intensity = nMax;
			for (int j = nMin; j < nMax; j++) {
				m_tf->getTF()[j].r = (cur_color[0] + valR) / 255.0;
				m_tf->getTF()[j].g = (cur_color[1] + valG) / 255.0;
				m_tf->getTF()[j].b = (cur_color[2] + valB) / 255.0;
				valR += dR;
				valG += dG;
				valB += dB;
			}
			prev_intensity = cur_intensity;
			prev_color = cur_color;
			last_color = cur_color;
			i++;


		}


	}

	for (int k = last_intensity; k < (m_max_val - m_min_val); k++) {


		std::vector<int> cur_color = last_color;

		m_tf->getTF()[k].r = cur_color[0] / 255.0;
		m_tf->getTF()[k].g = cur_color[1] / 255.0;
		m_tf->getTF()[k].b = cur_color[2] / 255.0;
	}


	//Pre-Integrated Classification (1D - Approximated ver.)
	for (int i = 1; i < tf_width; i++) {
		m_tf->getTF()[i].r += m_tf->getTF()[i - 1].r;
		m_tf->getTF()[i].g += m_tf->getTF()[i - 1].g;
		m_tf->getTF()[i].b += m_tf->getTF()[i - 1].b;
	}

	glfwMakeContextCurrent(m_gl_mgr->getGLWindow());
	if (tex_tf)
		SAFE_DELETE_OBJECT(tex_tf);

	tex_tf = new Texture(m_tf->getTF(), tf_width, 2);
	glfwSwapBuffers(m_gl_mgr->getGLWindow());

}
void DVRRectMgr::initSamplingCount(int cnt) {
	m_render_resolution = cnt;
	m_sampling_cnt = glm::vec2(m_render_resolution, m_render_resolution);

}



void DVRRectMgr::setSamplingCount(int cnt, bool is_downsampling) {
	initSamplingCount(cnt);
	int screen_diff = 0;

	//screen_height 는 downsampling resolution 에 맞게 조절
	if (is_downsampling) {
		m_screen_width = std::ceil(cnt * m_panel_wh_ratio);
		m_screen_height = cnt;

		m_pixel_screen_len.x = (m_current_num_max * m_panel_wh_ratio) / (m_sampling_cnt.x - 1);
		m_pixel_screen_len.y = m_current_num_max / (m_sampling_cnt.y - 1);
		m_center_coordinate[0] = m_sampling_cnt.x / 2;
		m_center_coordinate[1] = m_sampling_cnt.y / 2;
	}
	else {

		m_screen_width = m_panel_width;
		m_screen_height = m_panel_height;

		m_pixel_screen_len.x = (m_current_num_max * m_panel_wh_ratio) / (m_sampling_cnt.x - 1);
		m_pixel_screen_len.y = m_current_num_max / (m_sampling_cnt.y - 1);
		m_center_coordinate[0] = m_sampling_cnt.x / 2;
		m_center_coordinate[1] = m_sampling_cnt.y / 2;
	}
}

DVRPacket DVRRectMgr::getDVRPacket() {
	DVRPacket packet;
	packet.lefttop_point = { P_screen.x, P_screen.y, P_screen.z };
	packet.normal_vector = { ns.x, ns.y, ns.z };
	packet.width_direction_vetor = { ws.x, ws.y, ws.z };
	packet.height_direction_vector = { hs.x, hs.y, hs.z };
	packet.pixel_space = { m_pixel_screen_len.x, m_pixel_screen_len.y, m_unit_ray_len };
	//int sampling_cnt = m_sampling_cnt.y;
	packet.m_axis_size = init_current_num_max / m_current_num_max;
	packet.histogram = dvr_histogram;
	packet.image_size = { m_screen_width, m_screen_height };
	return packet;
}

/* ========================================================================== */
/* 							   Private Functions							  */
/* ========================================================================== */

void DVRRectMgr::_setMaterials() {
	m_camera = glm::translate(glm::mat4(1), glm::vec3(-(m_render_resolution) / 2, -(m_render_resolution) / 2, 0.01 / 2));
	m_proj = glm::scale(glm::mat4(1), glm::vec3(2 / (float)(m_render_resolution), 2 / (float)m_render_resolution, 1 / (10000.0f - 0.01f)));

	m_gl_mgr->getVRShader()->use();

	m_gl_mgr->getVRShader()->setMat4("mvMatrix", m_camera);
	m_gl_mgr->getVRShader()->setMat4("projMatrix", m_proj);

	m_gl_mgr->getVRShader()->setBool("shader_on", m_shader_on);
	m_gl_mgr->getVRShader()->setFloat("m_specular", m_specular);
	m_gl_mgr->getVRShader()->setFloat("m_brightness", m_brightness);
	m_gl_mgr->getVRShader()->setInt("N_x", m_width);
	m_gl_mgr->getVRShader()->setInt("N_y", m_height);
	m_gl_mgr->getVRShader()->setInt("N_z", m_depth);
	m_gl_mgr->getVRShader()->setFloat("m_spacingX", m_spacingX);
	m_gl_mgr->getVRShader()->setFloat("m_spacingY", m_spacingY);
	m_gl_mgr->getVRShader()->setFloat("m_spacingZ", m_spacingZ);

	m_gl_mgr->getVRShader()->setVec3("P_screen", P_screen);
	m_gl_mgr->getVRShader()->setVec3("v_width", ws * m_pixel_screen_len.x);
	m_gl_mgr->getVRShader()->setVec3("v_height", hs * m_pixel_screen_len.y);
	m_gl_mgr->getVRShader()->setVec3("v_normal", ns * m_unit_ray_len);
	m_gl_mgr->getVRShader()->setFloat("fOffset", (float)m_min_val);
	//m_gl_mgr->getVRShader()->setFloat("max_intensity", (float)m_max_val);
	m_gl_mgr->getVRShader()->setBool("skipping", false);
	m_gl_mgr->getVRShader()->setInt("proj_type", m_projection_type);

	m_gl_mgr->getVRShader()->setInt("volume_data", 0);
	m_gl_mgr->getVRShader()->setInt("mask_data", 1);
	m_gl_mgr->getVRShader()->setInt("tex_tf", 2);
	m_gl_mgr->getVRShader()->setInt("sculpt_data", 3);

	m_gl_mgr->getVRShader()->setFloat("fDeltaX", (1.0 / m_width));
	m_gl_mgr->getVRShader()->setFloat("fDeltaY", (1.0 / m_height));
	m_gl_mgr->getVRShader()->setFloat("fDeltaZ", (1.0 / m_depth * (m_spacingX / m_spacingZ)));

	m_gl_mgr->getVRShader()->setFloat("x1", (float)(x1 - m_min_val));
	m_gl_mgr->getVRShader()->setFloat("y1", (float)y1 / 255);
	m_gl_mgr->getVRShader()->setFloat("x2", (float)(x2 - m_min_val));
	m_gl_mgr->getVRShader()->setFloat("y2", (float)y2 / 255);
	m_gl_mgr->getVRShader()->setFloat("x3", (float)(x3 - m_min_val));
	m_gl_mgr->getVRShader()->setFloat("y3", (float)y3 / 255);
	m_gl_mgr->getVRShader()->setFloat("x4", (float)(x4 - m_min_val));
	m_gl_mgr->getVRShader()->setFloat("y4", (float)y4 / 255);

	m_gl_mgr->setMaskShaderParam(VR);

	m_gl_mgr->getVRShader()->setBool("is_sculpt", m_is_sculpt || m_is_voi);
}

void DVRRectMgr::_setSculptTextureData() {
	if (m_is_sculpt || m_is_voi) {
#pragma omp parallel for
		for (CgipInt ijk = 0; ijk < m_height * m_width * m_depth; ijk++) {
			CgipInt i = ijk / (m_width * m_height);
			CgipInt j = (ijk % (m_width * m_height)) / m_width;
			CgipInt k = (ijk % (m_width * m_height)) % m_width;
			glm::vec3 vol_point = glm::vec3((k)*m_spacingX, (j)*m_spacingY, (i)*m_spacingZ);

			glm::vec3 v = vol_point - P_screen;
			float dist = glm::dot(v, glm::normalize(ns));

			glm::vec3 point = vol_point - dist * glm::normalize(ns);
			glm::vec3 point_t_origin = point - P_screen;

			int x_len = glm::dot(point_t_origin, glm::normalize(ws)) / m_pixel_screen_len.x;
			int y_len = glm::dot(point_t_origin, glm::normalize(hs)) / m_pixel_screen_len.y;

			if (x_len < 0 || x_len >= m_render_resolution || y_len < 0 || y_len >= m_render_resolution)
				continue;

			if (sculpt_plane->getPixelValue(x_len, y_len) > 0 && m_is_sculpt) {
				m_tex_sculpt[m_width * m_height * i + m_width * j + k] = 1;
			}

			if (sculpt_plane->getPixelValue(x_len, y_len) == 0 && m_is_voi) {
				m_tex_sculpt[m_width * m_height * i + m_width * j + k] = 1;
			}
		}
	}
	else {
#pragma omp parallel for
		for (CgipInt ijk = 0; ijk < m_height * m_width * m_depth; ijk++) {
			CgipInt i = ijk / (m_width * m_height);
			CgipInt j = (ijk % (m_width * m_height)) / m_width;
			CgipInt k = (ijk % (m_width * m_height)) % m_width;

			m_tex_sculpt[m_width * m_height * i + m_width * j + k] = 0;
		}
	}

	glfwMakeContextCurrent(m_gl_mgr->getGLWindow());
	if (sculpt_texture)
		SAFE_DELETE_OBJECT(sculpt_texture);

	sculpt_texture = new Texture(m_tex_sculpt, m_width, m_height, m_depth, 3);
	glfwSwapBuffers(m_gl_mgr->getGLWindow());
}


static int ncompare(const void* a, const void* b) { return *(const int*)a - *(const int*)b; }
void DVRRectMgr::_HoleFilling(std::vector<cgip::CgipPoint> points, CgipShort pval, cgip::CgipPlane* plane) {
	int nodes, pixelY, i, j;
	int nPointCnt = points.size();
	int* nodeX = SAFE_ALLOC_1D(int, nPointCnt);

	// Loop through the rows of the image
//#pragma omp parallel for
	for (pixelY = 0; pixelY <= m_render_resolution; pixelY++)
	{
		nodes = 0;
		memset(nodeX, 0, sizeof(int) * nPointCnt);
		for (i = 0, j = nPointCnt - 1; i < nPointCnt; j = i++)
			if ((points[i].y() > pixelY) != (points[j].y() > pixelY))
				nodeX[nodes++] = (int)((points[i].x() + (pixelY - points[i].y()) / (points[j].y() - points[i].y()) * (points[j].x() - points[i].x())));

		std::qsort(nodeX, nodes, sizeof(int), ncompare);

		// Set map values for points between the node pairs to 1
		for (i = 0; i < nodes; i += 2)
		{
			if (nodeX[i] > m_render_resolution)	break;
			if (nodeX[i + 1] > 0)
			{
				nodeX[i] = std::max(0, nodeX[i]);
				nodeX[i + 1] = std::min(nodeX[i + 1], m_render_resolution);
				for (j = nodeX[i]; j < nodeX[i + 1]; j++)
					plane->setPixelValue(j, pixelY, pval);
			}
		}
	}
	SAFE_DELETE_OBJECT(nodeX);
}

void DVRRectMgr::setShaderStatus(int status) {
	if (status) {
		m_shader_on = true;
	}
	else {
		m_shader_on = false;
	}
}

void DVRRectMgr::setLighting(float specular_val, float brightness_val) {
	m_specular = specular_val;
	m_brightness = brightness_val;
}

void DVRRectMgr::sampleByPanel(int panel_width, int panel_height) {
	m_screen_width = panel_width;
	m_screen_height = panel_height;
	m_sampling_cnt = glm::vec2(m_screen_width, m_screen_height);

	*m_dvr = std::move(cgip::CgipPlane(m_screen_width, m_screen_height));
	m_gl_mgr->setVert(m_sampling_cnt.x, m_sampling_cnt.y);
}