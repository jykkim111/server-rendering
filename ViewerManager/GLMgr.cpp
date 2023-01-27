#include "GLMgr.h"

GLMgr::GLMgr() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	if (m_gl_window) {
		glfwDestroyWindow(m_gl_window);
	}


	m_gl_window = glfwCreateWindow(m_screen_size, m_screen_size, "Rendering", NULL, NULL);
	if (m_gl_window == NULL)
	{
		const char* description;
		glfwGetError(&description);

		if (description) {
			QMessageBox glfw_error_box;
			//std::cout << "Failed to create GLFW window" << std::endl;
			glfw_error_box.setText("Error : Failed to create GLFW window\n " + QString(description) + "\n Please try updating your graphic drivers");
			glfw_error_box.exec();
		}
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(m_gl_window);



	// tell GLFW to capture our mouse
	//glfwSetInputMode(m_gl_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	//Check OpenGL Version
	std::string gl_version = (const char*)glGetString(GL_VERSION);
	std::string glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	std::cout << "OpenGL Version : " << stof(gl_version) << std::endl;
	std::cout << "OpenGL GLSL Version : " << glsl_version << std::endl;

	//if (stof(gl_version) < 3.3) {
	//	QMessageBox gl_version_warning;
	//	gl_version_warning.setText("Please update your graphics driver so that OpenGL 3.3 or above is supported");
	//	gl_version_warning.exec();
	//}
	
	SAFE_DELETE_OBJECT(m_vr_shader);
	SAFE_DELETE_OBJECT(m_mpr_shader);
	SAFE_DELETE_OBJECT(m_cpr_shader);
	SAFE_DELETE_OBJECT(m_cpr_path_shader);


	m_vr_shader = new Shader("./shaders/ray_casting.vs", "./shaders/ray_casting.fs");
	glBindAttribLocation(m_vr_shader->ID, 0, "vert_in");

	m_mpr_shader = new Shader("./shaders/mpr.vs", "./shaders/mpr.fs");
	glBindAttribLocation(m_mpr_shader->ID, 1, "vert_in");

	m_cpr_shader = new Shader("./shaders/cpr.vs", "./shaders/cpr.fs");
	glBindAttribLocation(m_cpr_shader->ID, 2, "vert_in");
	
	m_cpr_path_shader = new Shader("./shaders/cpr_path.vs", "./shaders/cpr_path.fs");
	glBindAttribLocation(m_cpr_path_shader->ID, 3, "vert_in");

	m_vao = new VAO();
}

GLMgr::~GLMgr() {
	if (m_gl_window) {
		glfwDestroyWindow(m_gl_window);
	}
	SAFE_DELETE_OBJECT(m_vr_shader);
	SAFE_DELETE_OBJECT(m_mpr_shader);
	SAFE_DELETE_OBJECT(m_cpr_shader);
	SAFE_DELETE_OBJECT(m_cpr_path_shader);
	SAFE_DELETE_OBJECT(m_vao);
	SAFE_DELETE_OBJECT(data_texture);
	SAFE_DELETE_OBJECT(mask_texture);
}

void GLMgr::init_GL(VolumeMgr* vol_mgr, MaskMgr* mask_mgr) {
	// Prevent assigning nullptr
	// assert(vol_mgr != nullptr && mask_mgr != nullptr);

	m_volume_mgr = vol_mgr;
	m_mask_mgr = mask_mgr;

	m_width = m_volume_mgr->getWidth();
	m_height = m_volume_mgr->getHeight();
	m_depth = m_volume_mgr->getDepth();

	m_spacingX = m_volume_mgr->getSpacingX();
	m_spacingY = m_volume_mgr->getSpacingY();
	m_spacingZ = m_volume_mgr->getSpacingZ();

	//m_min_val = m_volume_mgr->getMinVal();
	//m_max_val = m_volume_mgr->getMaxVal();

	m_min_val = -2048;
	m_max_val = m_volume_mgr->getMaxVal();

	m_window_center = m_volume_mgr->getDefaultWindowCenter();
	m_window_width = m_volume_mgr->getDefaultWindowWidth();

	widthMM = m_volume_mgr->getWidthMM();
	heightMM = m_volume_mgr->getHeightMM();
	depthMM = m_volume_mgr->getDepthMM();

	setVert(m_screen_size, m_screen_size);

	setDataTexture();
	setMaskTexture();
}

void GLMgr::setVert(int sampling_cnt_x, int sampling_cnt_y) {
	std::vector<GLfloat> data_vertex;
	data_vertex.resize(sampling_cnt_x * sampling_cnt_y * 3 * 6);
	int m_count = 0;
	for (int j = 0; j < sampling_cnt_x; j++) {
		for (int i = 0; i < sampling_cnt_y; i++) {
			GLfloat* p = data_vertex.data() + m_count;

			*p++ = i;
			*p++ = j;
			*p++ = 0;
			*p++ = i + 1;
			*p++ = j;
			*p++ = 0;
			*p++ = i;
			*p++ = j + 1;
			*p++ = 0;

			*p++ = i + 1;
			*p++ = j;
			*p++ = 0;
			*p++ = i + 1;
			*p++ = j + 1;
			*p++ = 0;
			*p++ = i;
			*p++ = j + 1;
			*p++ = 0;

			m_count += 18;
		}
	}
	glfwMakeContextCurrent(m_gl_window);
	m_vao->deleteVertex();
	m_vao->setVertex(data_vertex);
	glfwSwapBuffers(m_gl_window);
}

void GLMgr::setDataTexture() {
	unsigned short* tex_data = SAFE_ALLOC_2D(unsigned short, m_width * m_height, m_depth);
#pragma omp parallel for
	for (CgipInt ijk = 0; ijk < m_height * m_width * m_depth; ijk++) {
		CgipInt i = ijk / (m_width * m_height);
		CgipInt j = (ijk % (m_width * m_height)) / m_width;
		CgipInt k = (ijk % (m_width * m_height)) % m_width;

		short temp = m_volume_mgr->getRawPtr()->getRawPtr()[i][j * m_width + k];
		if (temp < -2048 || temp > m_max_val) {
			temp = m_min_val;
		}
		temp -= m_min_val;		 // move to 0~4095

		tex_data[m_width * m_height * i + m_width * j + k] = temp;
	}

	glfwMakeContextCurrent(m_gl_window);
	SAFE_DELETE_OBJECT(data_texture);
	data_texture = new Texture(tex_data, m_width, m_height, m_depth, 0);
	glfwSwapBuffers(m_gl_window);

	SAFE_DELETE_ARRAY(tex_data);
}

void GLMgr::setMaskTexture() {
	cgip::CgipMask mask = m_mask_mgr->getViewingVolume(m_width, m_height, m_depth);
	unsigned char* tex_data = SAFE_ALLOC_2D(unsigned char, m_width * m_height, m_depth);

#pragma omp parallel for
	for (CgipInt ijk = 0; ijk < m_height * m_width * m_depth; ijk++) {
		CgipInt i = ijk / (m_width * m_height);
		CgipInt j = (ijk % (m_width * m_height)) / m_width;
		CgipInt k = (ijk % (m_width * m_height)) % m_width;

		tex_data[m_width * m_height * i + m_width * j + k] = mask.getVoxelValue(k, j, i);
	}

	glfwMakeContextCurrent(m_gl_window);
	SAFE_DELETE_OBJECT(mask_texture);
	mask_texture = new Texture(tex_data, m_width, m_height, m_depth, 1);
	glfwSwapBuffers(m_gl_window);

	SAFE_DELETE_ARRAY(tex_data);
}

void GLMgr::setMaskShaderParam(RenderingType rendering_type) {
	if (rendering_type == MPR) {
		glm::vec3 v[9];
		int vis[9];
		for (int i = 1; i < G_MAX_MASK_NUM; i++) {
			//glUniform1fv(glGetUniformLocation(program, "v"), 10, v);
			QString mask_val = QString::fromStdString(m_mask_mgr->getColor(i));
			float r = QColor(mask_val).red() / 255.0;
			float g = QColor(mask_val).green() / 255.0;
			float b = QColor(mask_val).blue() / 255.0;
			v[i] = glm::vec3(b, r, g);

			vis[i] = (int)m_mask_mgr->getVisibility(i);
		}

		m_mpr_shader->setInt("mask_visibility", 9, vis);
		m_mpr_shader->setVec3("color_palette", 9, v[0]);
		m_mpr_shader->setFloat("mask_opacity", m_mask_mgr->getOpacity() / 10.0f);
	}
	else if (rendering_type == VR) {
		glm::vec4 v[9];
		int vis[9];
		for (int i = 1; i < G_MAX_MASK_NUM; i++) {
			//glUniform1fv(glGetUniformLocation(program, "v"), 10, v);
			QString mask_val = QString::fromStdString(m_mask_mgr->getColor(i));
			float r = QColor(mask_val).red() / 255.0;
			float g = QColor(mask_val).green() / 255.0;
			float b = QColor(mask_val).blue() / 255.0;
			v[i] = glm::vec4(b, r, g, m_mask_mgr->getOpacity() / 10.0f);

			vis[i] = (int)m_mask_mgr->getVisibility(i);
		}
		m_vr_shader->setInt("mask_visibility", 9, vis);
		m_vr_shader->setVec4("color_palette", 9, v[0]);
	}
}

void GLMgr::setDownsampledDataTexture(int len) {
	const int sizeX = m_width / len;
	const int sizeY = m_height / len;
	const int sizeZ = m_depth / len;
	unsigned short* tex_data = SAFE_ALLOC_2D(unsigned short, sizeX * sizeY, sizeZ);

	int sum;
	short avg;
	const int num = len * len * len;
#pragma omp parallel for
	for (CgipInt ijk = 0; ijk < sizeX * sizeY * sizeZ; ++ijk) {
		CgipInt i = ijk / (sizeX * sizeY);
		CgipInt j = (ijk % (sizeX * sizeY)) / sizeX;
		CgipInt k = (ijk % (sizeX * sizeY)) % sizeX;

		sum = 0;
		for (int m = i * len; m < i * len + len; m++)
			for (int n = j * len; n < j * len + len; n++)
				for (int o = k * len; o < k * len + len; o++) {
					if (m < m_depth && n < m_height && o < m_width)
						sum += m_volume_mgr->getRawPtr()->getRawPtr()[m][n * m_width + o];
				}
		avg = sum / num;
		tex_data[i * sizeX * sizeY + j * sizeX + k] = avg - m_min_val;
	}
	glfwMakeContextCurrent(m_gl_window);
	SAFE_DELETE_OBJECT(data_texture);
	data_texture = new Texture(tex_data, sizeX, sizeY, sizeZ, 0);
	glfwSwapBuffers(m_gl_window);

	SAFE_DELETE_ARRAY(tex_data);
}

void GLMgr::setDownsampledMaskTexture(int len) {
	cgip::CgipMask mask = m_mask_mgr->getViewingVolume(m_width, m_height, m_depth);

	const int sizeX = m_width / len;
	const int sizeY = m_height / len;
	const int sizeZ = m_depth / len;
	unsigned short* tex_data = SAFE_ALLOC_2D(unsigned short, sizeX * sizeY, sizeZ);

	int sum;
	short avg;
	const int num = len * len * len;
#pragma omp parallel for
	for (CgipInt ijk = 0; ijk < sizeX * sizeY * sizeZ; ++ijk) {
		CgipInt i = ijk / (sizeX * sizeY);
		CgipInt j = (ijk % (sizeX * sizeY)) / sizeX;
		CgipInt k = (ijk % (sizeX * sizeY)) % sizeX;

		sum = 0;
		for (int m = i * len; m < i * len + len; m++)
			for (int n = j * len; n < j * len + len; n++)
				for (int o = k * len; o < k * len + len; o++) {
					if (m < m_depth && n < m_height && o < m_width)
						sum += mask.getVoxelValue(o, n, m);
				}
		avg = sum / num;
		tex_data[i * sizeX * sizeY + j * sizeX + k] = avg;
	}

	glfwMakeContextCurrent(m_gl_window);
	SAFE_DELETE_OBJECT(mask_texture);
	mask_texture = new Texture(tex_data, sizeX, sizeY, sizeZ, 1);
	glfwSwapBuffers(m_gl_window);

	SAFE_DELETE_ARRAY(tex_data);
}