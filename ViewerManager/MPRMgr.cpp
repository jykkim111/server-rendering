#include "MPRMgr.h"

#include <Eigen/Geometry>

MPRMgr::MPRMgr()
{
	// Initialize with empty planes
	m_axial = new cgip::CgipPlane;
	m_coronal = new cgip::CgipPlane;
	m_sagittal = new cgip::CgipPlane;
	m_raw = new cgip::CgipPlane;
}

MPRMgr::~MPRMgr() {
	SAFE_DELETE_OBJECT(m_axial);
	SAFE_DELETE_OBJECT(m_coronal);
	SAFE_DELETE_OBJECT(m_sagittal);
	SAFE_DELETE_OBJECT(m_raw);
}

/* set slice sampling counts */
void MPRMgr::initSamplingCount(int w_cnt, int h_cnt, int c) {
	m_w_sampling_cnt = w_cnt;
	m_h_sampling_cnt = h_cnt;


	//*m_axial = std::move(cgip::CgipPlane(w_cnt, h_cnt, c));
	//*m_coronal = std::move(cgip::CgipPlane(w_cnt, h_cnt, c));
	//*m_sagittal = std::move(cgip::CgipPlane(w_cnt, h_cnt, c));

	//m_gl_mgr->setVert(m_w_sampling_cnt, m_h_sampling_cnt);
}

void MPRMgr::setSamplingCount(int w_cnt, int h_cnt, int c, bool is_downsampling) {
	initSamplingCount(w_cnt, h_cnt, c);
	m_axial_w_sampling_len = m_axial_size / (w_cnt - 1);
	m_axial_h_sampling_len = m_axial_size / (h_cnt - 1);
	m_coronal_w_sampling_len = m_coronal_size / (w_cnt - 1);
	m_coronal_h_sampling_len = m_coronal_size / (h_cnt - 1);
	m_sagittal_w_sampling_len = m_sagittal_size / (w_cnt - 1);
	m_sagittal_h_sampling_len = m_sagittal_size / (h_cnt - 1);



	//m_gl_mgr->setVert(m_w_sampling_cnt, m_h_sampling_cnt);
}

/* get slice info */
std::tuple<int, int, float, float, cgip::CgipVector, cgip::CgipVector, cgip::CgipPoint> MPRMgr::getAxialInfo() {
	return {
		m_w_sampling_cnt, m_h_sampling_cnt,
		m_axial_w_sampling_len, m_axial_h_sampling_len,
		m_axial_w_dir, m_axial_h_dir, m_axial_topleft_pt
	};
}
std::tuple<int, int, float, float, cgip::CgipVector, cgip::CgipVector, cgip::CgipPoint> MPRMgr::getCoronalInfo() {
	return {
		m_w_sampling_cnt, m_h_sampling_cnt,
		m_coronal_w_sampling_len, m_coronal_h_sampling_len,
		m_coronal_w_dir, m_coronal_h_dir, m_coronal_topleft_pt
	};
}
std::tuple<int, int, float, float, cgip::CgipVector, cgip::CgipVector, cgip::CgipPoint> MPRMgr::getSagittalInfo() {
	return {
		m_w_sampling_cnt, m_h_sampling_cnt,
		m_sagittal_w_sampling_len, m_sagittal_h_sampling_len,
		m_sagittal_w_dir, m_sagittal_h_dir, m_sagittal_topleft_pt
	};
}

SlicePacket MPRMgr::getSlicePacket(SliceType type) {
	SlicePacket packet;
	packet.slice_line = getSliceLineInfo(type);
	packet.thickness = getThicknessInfo(type);
	packet.image_num = getSliceNumInfo(type);
	packet.panning = getPanningInfo(type);
	packet.center_point = { m_axis_center_pt.x() ,  m_axis_center_pt.y(),  m_axis_center_pt.z() };
	packet.image_size = { m_w_sampling_cnt, m_h_sampling_cnt };

	switch (type) {
	case AXIAL_SLICE:
		packet.lefttop_point = { m_axial_topleft_pt.x() ,  m_axial_topleft_pt.y(),  m_axial_topleft_pt.z() };
		packet.normal_vector = { m_axial_n_dir.x() ,  m_axial_n_dir.y(),  m_axial_n_dir.z() };
		packet.width_direction_vetor = { m_axial_w_dir.x() ,  m_axial_w_dir.y(),  m_axial_w_dir.z() };
		packet.height_direction_vector = { m_axial_h_dir.x() ,  m_axial_h_dir.y(),  m_axial_h_dir.z() };
		packet.pixel_space = { m_axial_w_sampling_len, m_axial_h_sampling_len };
		packet.m_axis_size = init_axial_size / m_axial_size;

		break;
	case CORONAL_SLICE:
		packet.lefttop_point = { m_coronal_topleft_pt.x() ,  m_coronal_topleft_pt.y(),  m_coronal_topleft_pt.z() };
		packet.normal_vector = { m_coronal_n_dir.x() ,  m_coronal_n_dir.y(),  m_coronal_n_dir.z() };
		packet.width_direction_vetor = { m_coronal_w_dir.x() ,  m_coronal_w_dir.y(),  m_coronal_w_dir.z() };
		packet.height_direction_vector = { m_coronal_h_dir.x() ,  m_coronal_h_dir.y(),  m_coronal_h_dir.z() };
		packet.pixel_space = { m_coronal_w_sampling_len, m_coronal_h_sampling_len };
		packet.m_axis_size = init_coronal_size / m_coronal_size;
		break;
	case SAGITTAL_SLICE:
		packet.lefttop_point = { m_sagittal_topleft_pt.x() ,  m_sagittal_topleft_pt.y(),  m_sagittal_topleft_pt.z() };
		packet.normal_vector = { m_sagittal_n_dir.x() ,  m_sagittal_n_dir.y(),  m_sagittal_n_dir.z() };
		packet.width_direction_vetor = { m_sagittal_w_dir.x() ,  m_sagittal_w_dir.y(),  m_sagittal_w_dir.z() };
		packet.height_direction_vector = { m_sagittal_h_dir.x() ,  m_sagittal_h_dir.y(),  m_sagittal_h_dir.z() };
		packet.pixel_space = { m_sagittal_w_sampling_len, m_sagittal_h_sampling_len };
		packet.m_axis_size = init_sagittal_size / m_sagittal_size;
		break;
	}

	return packet;
}

/* set planes */
void MPRMgr::setAxial()
{
	int cnt = 0;
	float dist = 0;

	bool is_thickness = !(int(m_axial_thickness / 2) < 1);
	if (is_thickness) {
		cnt = int(m_axial_thickness / 2);
		dist = _get_dist_next_slice(1, m_axis_center_pt.z(), m_gl_mgr->getSpacingZ());
	}

	*m_axial = std::move(_setMPR(m_axial_topleft_pt, m_axial_w_dir, m_axial_h_dir, m_axial_n_dir,
		m_w_sampling_cnt, m_h_sampling_cnt, cnt,
		m_axial_w_sampling_len, m_axial_h_sampling_len, dist, is_thickness, m_axial_window_width, m_axial_window_level, m_axial_inverse));
}
void MPRMgr::setCoronal()
{
	int cnt = 0;
	float dist = 0;

	bool is_thickness = !(int(m_coronal_thickness / 2) < 1);
	if (is_thickness) {
		cnt = int(m_coronal_thickness / 2);
		dist = _get_dist_next_slice(1, m_axis_center_pt.y(), m_gl_mgr->getSpacingY());
	}

	*m_coronal = std::move(_setMPR(m_coronal_topleft_pt, m_coronal_w_dir, m_coronal_h_dir, m_coronal_n_dir,
		m_w_sampling_cnt, m_h_sampling_cnt, cnt,
		m_coronal_w_sampling_len, m_coronal_h_sampling_len, dist, is_thickness, m_coronal_window_width, m_coronal_window_level, m_coronal_inverse));
}
void MPRMgr::setSagittal()
{
	int cnt = 0;
	float dist = 0;

	bool is_thickness = !(int(m_sagittal_thickness / 2) < 1);
	if (is_thickness) {
		cnt = int(m_sagittal_thickness / 2);
		dist = _get_dist_next_slice(1, m_axis_center_pt.x(), m_gl_mgr->getSpacingX());
	}

	*m_sagittal = std::move(_setMPR(m_sagittal_topleft_pt, m_sagittal_w_dir, m_sagittal_h_dir, m_sagittal_n_dir,
		m_w_sampling_cnt, m_h_sampling_cnt, cnt,
		m_sagittal_w_sampling_len, m_sagittal_h_sampling_len, dist, is_thickness, m_sagittal_window_width, m_sagittal_window_level, m_sagittal_inverse));
}

void MPRMgr::setRawMPR(SliceType slice_type) {
	cgip::CgipPoint topleft_pt;
	int w_sampling_cnt, h_sampling_cnt;
	float w_sampling_len, h_sampling_len;

	float n_x, n_y, n_z, t;
	cgip::CgipPoint w1, w2, h1, h2;

	switch (slice_type) {
	case AXIAL_SLICE:
		std::tie(w1, w2) = _find_intersection_point(m_axis_center_pt, m_axial_w_dir);
		std::tie(h1, h2) = _find_intersection_point(m_axis_center_pt, m_axial_h_dir);
		if ((m_axial_w_dir.x() - m_axial_h_dir.x()) != 0) {
			t = (w1.x() - h1.x()) / (m_axial_w_dir.x() - m_axial_h_dir.x());
		}
		else if ((m_axial_w_dir.y() - m_axial_h_dir.y()) != 0) {
			t = (w1.y() - h1.y()) / (m_axial_w_dir.y() - m_axial_h_dir.y());
		}
		else if ((m_axial_w_dir.z() - m_axial_h_dir.z()) != 0) {
			t = (w1.z() - h1.z()) / (m_axial_w_dir.z() - m_axial_h_dir.z());
		}
		n_x = w1.x() + m_axial_h_dir.x() * t;
		n_y = w1.y() + m_axial_h_dir.y() * t;
		n_z = w1.z() + m_axial_h_dir.z() * t;
		topleft_pt.setXYZ(n_x, n_y, n_z);
		w_sampling_cnt = ceil(sqrt(pow(w1.x() - w2.x(), 2) + pow(w1.y() - w2.y(), 2) + pow(w1.z() - w2.z(), 2)));
		h_sampling_cnt = ceil(sqrt(pow(h1.x() - h2.x(), 2) + pow(h1.y() - h2.y(), 2) + pow(h1.z() - h2.z(), 2)));

		getViewingPlane(topleft_pt, m_axial_w_dir, m_axial_h_dir,
			w_sampling_cnt, h_sampling_cnt,
			1.0, 1.0);
		break;

	case CORONAL_SLICE:
		std::tie(w1, w2) = _find_intersection_point(m_axis_center_pt, m_coronal_w_dir);
		std::tie(h1, h2) = _find_intersection_point(m_axis_center_pt, m_coronal_h_dir);
		if ((m_coronal_w_dir.x() - m_coronal_h_dir.x()) != 0) {
			t = (w1.x() - h1.x()) / (m_coronal_w_dir.x() - m_coronal_h_dir.x());
		}
		else if ((m_coronal_w_dir.y() - m_coronal_h_dir.y()) != 0) {
			t = (w1.y() - h1.y()) / (m_coronal_w_dir.y() - m_coronal_h_dir.y());
		}
		else if ((m_coronal_w_dir.z() - m_coronal_h_dir.z()) != 0) {
			t = (w1.z() - h1.z()) / (m_coronal_w_dir.z() - m_coronal_h_dir.z());
		}
		n_x = w1.x() + m_coronal_h_dir.x() * t;
		n_y = w1.y() + m_coronal_h_dir.y() * t;
		n_z = w1.z() + m_coronal_h_dir.z() * t;
		topleft_pt.setXYZ(n_x, n_y, n_z);

		w_sampling_cnt = ceil(sqrt(pow(w1.x() - w2.x(), 2) + pow(w1.y() - w2.y(), 2) + pow(w1.z() - w2.z(), 2)));
		h_sampling_cnt = ceil(sqrt(pow(h1.x() - h2.x(), 2) + pow(h1.y() - h2.y(), 2) + pow(h1.z() - h2.z(), 2)));

		getViewingPlane(topleft_pt, m_axial_w_dir, m_axial_h_dir,
			w_sampling_cnt, h_sampling_cnt,
			1.0, 1.0);
		break;

	case SAGITTAL_SLICE:
		std::tie(w1, w2) = _find_intersection_point(m_axis_center_pt, m_sagittal_w_dir);
		std::tie(h1, h2) = _find_intersection_point(m_axis_center_pt, m_sagittal_h_dir);
		if ((m_sagittal_w_dir.x() - m_sagittal_h_dir.x()) != 0) {
			t = (w1.x() - h1.x()) / (m_sagittal_w_dir.x() - m_sagittal_h_dir.x());
		}
		else if ((m_sagittal_w_dir.y() - m_sagittal_h_dir.y()) != 0) {
			t = (w1.y() - h1.y()) / (m_sagittal_w_dir.y() - m_sagittal_h_dir.y());
		}
		else if ((m_sagittal_w_dir.z() - m_sagittal_h_dir.z()) != 0) {
			t = (w1.z() - h1.z()) / (m_sagittal_w_dir.z() - m_sagittal_h_dir.z());
		}
		n_x = w1.x() + m_sagittal_h_dir.x() * t;
		n_y = w1.y() + m_sagittal_h_dir.y() * t;
		n_z = w1.z() + m_sagittal_h_dir.z() * t;
		topleft_pt.setXYZ(n_x, n_y, n_z);

		w_sampling_cnt = ceil(sqrt(pow(w1.x() - w2.x(), 2) + pow(w1.y() - w2.y(), 2) + pow(w1.z() - w2.z(), 2)));
		h_sampling_cnt = ceil(sqrt(pow(h1.x() - h2.x(), 2) + pow(h1.y() - h2.y(), 2) + pow(h1.z() - h2.z(), 2)));

		getViewingPlane(topleft_pt, m_axial_w_dir, m_axial_h_dir,
			w_sampling_cnt, h_sampling_cnt,
			1.0, 1.0);
		break;
	}
}

void MPRMgr::init_MPR(GLMgr* gl_mgr, VolumeMgr* vol_mgr)
{
	m_gl_mgr = gl_mgr;
	m_vol_mgr = vol_mgr;

	// set windowing
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

	init_position();
}

void MPRMgr::init_position(int axial_loc)
{
	float width_mm = m_gl_mgr->getWidthMM();
	float height_mm = m_gl_mgr->getHeightMM();
	float depth_mm = m_gl_mgr->getDepthMM();

	// set initial location of axis center
	float axis_center_z;
	if (axial_loc < 0)
		axis_center_z = m_gl_mgr->getDepth() / 2;
	else
		axis_center_z = axial_loc;

	m_axis_center_pt = cgip::CgipPoint(
		(m_gl_mgr->getWidth() / 2) * m_gl_mgr->getSpacingX(),
		(m_gl_mgr->getHeight() / 2) * m_gl_mgr->getSpacingY(),
		axis_center_z * m_gl_mgr->getSpacingZ());

	// set initial slice size && location
	m_axial_size = std::max(width_mm, height_mm) + SLICE_MARGIN;
	m_coronal_size = std::max(width_mm, depth_mm) + SLICE_MARGIN;
	m_sagittal_size = std::max(depth_mm, height_mm) + SLICE_MARGIN;

	init_axial_size = m_axial_size;
	init_coronal_size = m_coronal_size;
	init_sagittal_size = m_sagittal_size;

	m_axial_w_sampling_len = m_axial_size / (m_w_sampling_cnt - 1);
	m_axial_h_sampling_len = m_axial_size / (m_h_sampling_cnt - 1);
	m_coronal_w_sampling_len = m_coronal_size / (m_w_sampling_cnt - 1);
	m_coronal_h_sampling_len = m_coronal_size / (m_h_sampling_cnt - 1);
	m_sagittal_w_sampling_len = m_sagittal_size / (m_w_sampling_cnt - 1);
	m_sagittal_h_sampling_len = m_sagittal_size / (m_h_sampling_cnt - 1);

	// set initial slice directions
	m_axial_n_dir = cgip::CgipVector(0, 0, -1);
	m_axial_w_dir = cgip::CgipVector(1, 0, 0);
	m_axial_h_dir = cgip::CgipVector(0, 1, 0);
	m_coronal_n_dir = cgip::CgipVector(0, 1, 0);
	m_coronal_w_dir = cgip::CgipVector(1, 0, 0);
	m_coronal_h_dir = cgip::CgipVector(0, 0, 1);
	m_sagittal_n_dir = cgip::CgipVector(1, 0, 0);
	m_sagittal_w_dir = cgip::CgipVector(0, 1, 0);
	m_sagittal_h_dir = cgip::CgipVector(0, 0, 1);

	// set initial angles of slice lines on each plane
	m_angle_on_axial = 0;
	m_angle_on_coronal = 0;
	m_angle_on_sagittal = 0;

	m_axial_thickness = 0;
	m_coronal_thickness = 0;
	m_sagittal_thickness = 0;

	_resetTopLeftPt();


	glm::mat4 m_init_rotation = m_vol_mgr->getInitRotationMat();
	glm::mat4 m_init_shear = m_vol_mgr->getInitShearMat();

	Eigen::Vector3f rot_center;
	Eigen::Affine3f rot_mat, trans_rot_mat, shear_mat, trans_shear_mat;

	rot_center = m_axis_center_pt.toEigen();

	Eigen::Matrix4f init_rot_mat;
	init_rot_mat << m_init_rotation[0][0], m_init_rotation[1][0], m_init_rotation[2][0], m_init_rotation[3][0],
		m_init_rotation[0][1], m_init_rotation[1][1], m_init_rotation[2][1], m_init_rotation[3][1],
		m_init_rotation[0][2], m_init_rotation[1][2], m_init_rotation[2][2], m_init_rotation[3][2],
		m_init_rotation[0][3], m_init_rotation[1][3], m_init_rotation[2][3], m_init_rotation[3][3];


	std::cout << "initial_rotation_MPR: " << init_rot_mat << std::endl;

	rot_mat.matrix() = init_rot_mat;

	trans_rot_mat = Eigen::Translation3f(rot_center) * rot_mat * Eigen::Translation3f(-rot_center);

	m_axial_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_axial_topleft_pt.toEigen(1)).head<3>());
	m_axial_w_dir = cgip::CgipVector((rot_mat * m_axial_w_dir.toEigen(1)).head<3>());
	m_axial_h_dir = cgip::CgipVector((rot_mat * m_axial_h_dir.toEigen(1)).head<3>());
	m_axial_n_dir = cgip::CgipVector((rot_mat * m_axial_n_dir.toEigen(1)).head<3>());

	m_coronal_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_coronal_topleft_pt.toEigen(1)).head<3>());
	m_coronal_w_dir = cgip::CgipVector((rot_mat * m_coronal_w_dir.toEigen(1)).head<3>());
	m_coronal_h_dir = cgip::CgipVector((rot_mat * m_coronal_h_dir.toEigen(1)).head<3>());
	m_coronal_n_dir = cgip::CgipVector((rot_mat * m_coronal_n_dir.toEigen(1)).head<3>());

	m_sagittal_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_sagittal_topleft_pt.toEigen(1)).head<3>());
	m_sagittal_w_dir = cgip::CgipVector((rot_mat * m_sagittal_w_dir.toEigen(1)).head<3>());
	m_sagittal_h_dir = cgip::CgipVector((rot_mat * m_sagittal_h_dir.toEigen(1)).head<3>());
	m_sagittal_n_dir = cgip::CgipVector((rot_mat * m_sagittal_n_dir.toEigen(1)).head<3>());




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
	m_axial_window_level = default_window_center;
	m_axial_window_width = default_window_width;
	m_sagittal_window_level = default_window_center;
	m_sagittal_window_width = default_window_width;
	m_coronal_window_level = default_window_center;
	m_coronal_window_width = default_window_width;
}

void MPRMgr::init_position(SliceType slice_type, int axial_loc)
{
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

	switch (slice_type) {
	case AXIAL_SLICE:
		m_axis_center_pt = cgip::CgipPoint(
			m_axis_center_pt.x(),
			m_axis_center_pt.y(),
			axis_center_z * m_gl_mgr->getSpacingZ());

		m_axial_size = std::max(width_mm, height_mm) + SLICE_MARGIN;
		init_axial_size = m_axial_size;
		m_axial_w_sampling_len = m_axial_size / (m_w_sampling_cnt - 1);
		m_axial_h_sampling_len = m_axial_size / (m_h_sampling_cnt - 1);
		m_axial_n_dir = cgip::CgipVector(0, 0, -1);
		m_axial_w_dir = cgip::CgipVector(1, 0, 0);
		m_axial_h_dir = cgip::CgipVector(0, 1, 0);
		m_angle_on_axial = 0;
		m_axial_thickness = 0;
		m_axial_window_level = default_window_center;
		m_axial_window_width = default_window_width;
		m_axial_inverse = false;
		break;
	case SAGITTAL_SLICE:
		m_axis_center_pt = cgip::CgipPoint(
			(m_gl_mgr->getWidth() / 2) * m_gl_mgr->getSpacingX(),
			m_axis_center_pt.y(),
			m_axis_center_pt.z());

		m_sagittal_size = std::max(depth_mm, height_mm) + SLICE_MARGIN;
		init_sagittal_size = m_sagittal_size;
		m_sagittal_w_sampling_len = m_sagittal_size / (m_w_sampling_cnt - 1);
		m_sagittal_h_sampling_len = m_sagittal_size / (m_h_sampling_cnt - 1);
		m_sagittal_n_dir = cgip::CgipVector(1, 0, 0);
		m_sagittal_w_dir = cgip::CgipVector(0, 1, 0);
		m_sagittal_h_dir = cgip::CgipVector(0, 0, 1);
		m_angle_on_sagittal = 0;
		m_sagittal_thickness = 0;
		m_sagittal_window_level = default_window_center;
		m_sagittal_window_width = default_window_width;
		m_sagittal_inverse = false;
		break;
	case CORONAL_SLICE:
		m_axis_center_pt = cgip::CgipPoint(
			m_axis_center_pt.x(),
			(m_gl_mgr->getHeight() / 2) * m_gl_mgr->getSpacingY(),
			m_axis_center_pt.z());

		m_coronal_size = std::max(width_mm, depth_mm) + SLICE_MARGIN;
		init_coronal_size = m_coronal_size;
		m_coronal_w_sampling_len = m_coronal_size / (m_w_sampling_cnt - 1);
		m_coronal_h_sampling_len = m_coronal_size / (m_h_sampling_cnt - 1);
		m_coronal_n_dir = cgip::CgipVector(0, 1, 0);
		m_coronal_w_dir = cgip::CgipVector(1, 0, 0);
		m_coronal_h_dir = cgip::CgipVector(0, 0, 1);
		m_angle_on_coronal = 0;
		m_coronal_thickness = 0;
		m_coronal_window_level = default_window_center;
		m_coronal_window_width = default_window_width;
		m_coronal_inverse = false;
		break;
	}

	_resetTopLeftPt();


	glm::mat4 m_init_rotation = m_vol_mgr->getInitRotationMat();
	glm::mat4 m_init_shear = m_vol_mgr->getInitShearMat();

	Eigen::Vector3f rot_center;
	Eigen::Affine3f rot_mat, trans_rot_mat, shear_mat, trans_shear_mat;



	cgip::CgipPoint center_pt = cgip::CgipPoint(
		(m_gl_mgr->getWidth() / 2) * m_gl_mgr->getSpacingX(),
		(m_gl_mgr->getHeight() / 2) * m_gl_mgr->getSpacingY(),
		(m_gl_mgr->getDepth() / 2) * m_gl_mgr->getSpacingZ());

	rot_center = center_pt.toEigen();

	Eigen::Matrix4f init_rot_mat;
	init_rot_mat << m_init_rotation[0][0], m_init_rotation[1][0], m_init_rotation[2][0], m_init_rotation[3][0],
		m_init_rotation[0][1], m_init_rotation[1][1], m_init_rotation[2][1], m_init_rotation[3][1],
		m_init_rotation[0][2], m_init_rotation[1][2], m_init_rotation[2][2], m_init_rotation[3][2],
		m_init_rotation[0][3], m_init_rotation[1][3], m_init_rotation[2][3], m_init_rotation[3][3];


	std::cout << "initial_rotation_MPR: " << init_rot_mat << std::endl;

	rot_mat.matrix() = init_rot_mat;

	trans_rot_mat = Eigen::Translation3f(rot_center) * rot_mat * Eigen::Translation3f(-rot_center);

	switch (slice_type) {
	case AXIAL_SLICE:
		m_axial_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_axial_topleft_pt.toEigen(1)).head<3>());
		m_axial_w_dir = cgip::CgipVector((rot_mat * m_axial_w_dir.toEigen(1)).head<3>());
		m_axial_h_dir = cgip::CgipVector((rot_mat * m_axial_h_dir.toEigen(1)).head<3>());
		m_axial_n_dir = cgip::CgipVector((rot_mat * m_axial_n_dir.toEigen(1)).head<3>());
		break;
	case CORONAL_SLICE:
		m_coronal_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_coronal_topleft_pt.toEigen(1)).head<3>());
		m_coronal_w_dir = cgip::CgipVector((rot_mat * m_coronal_w_dir.toEigen(1)).head<3>());
		m_coronal_h_dir = cgip::CgipVector((rot_mat * m_coronal_h_dir.toEigen(1)).head<3>());
		m_coronal_n_dir = cgip::CgipVector((rot_mat * m_coronal_n_dir.toEigen(1)).head<3>());
		break;
	case SAGITTAL_SLICE:
		m_sagittal_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_sagittal_topleft_pt.toEigen(1)).head<3>());
		m_sagittal_w_dir = cgip::CgipVector((rot_mat * m_sagittal_w_dir.toEigen(1)).head<3>());
		m_sagittal_h_dir = cgip::CgipVector((rot_mat * m_sagittal_h_dir.toEigen(1)).head<3>());
		m_sagittal_n_dir = cgip::CgipVector((rot_mat * m_sagittal_n_dir.toEigen(1)).head<3>());
		break;
	}
}

void MPRMgr::_resetTopLeftPt()
{
	// reset topleft points according to axis center
	cgip::CgipPoint center_pt = cgip::CgipPoint(
		(m_gl_mgr->getWidth() / 2) * m_gl_mgr->getSpacingX(),
		(m_gl_mgr->getHeight() / 2) * m_gl_mgr->getSpacingY(),
		(m_gl_mgr->getDepth() / 2) * m_gl_mgr->getSpacingZ());

	m_axial_topleft_pt = center_pt + (m_axial_w_dir + m_axial_h_dir) * (-m_axial_size / 2);
	m_coronal_topleft_pt = center_pt + (m_coronal_w_dir + m_coronal_h_dir) * (-m_coronal_size / 2);
	m_sagittal_topleft_pt = center_pt + (m_sagittal_w_dir + m_sagittal_h_dir) * (-m_sagittal_size / 2);

}

bool MPRMgr::_is_outside(VolumeMgr* vol_mgr, cgip::CgipPoint p)
{
	return (p.x() < 0 || vol_mgr->getWidthMM() < p.x() ||
		p.y() < 0 || vol_mgr->getHeightMM() < p.y() ||
		p.z() < 0 || vol_mgr->getDepthMM() < p.z());
}

int MPRMgr::move_slice(SliceType slice_type, VolumeMgr* vol_mgr, float x, float y, bool h, bool v)
{
	cgip::CgipVector d_vec;
	cgip::CgipVector horizontal_n_dir, vertical_n_dir;
	cgip::CgipVector horizontal_d_vec, vertical_d_vec;
	cgip::CgipPoint new_axis_center;
	switch (slice_type) {
	case AXIAL_SLICE:
		new_axis_center = m_axial_topleft_pt + (x * m_axial_w_dir + y * m_axial_h_dir) * m_axial_size;
		horizontal_n_dir = m_coronal_n_dir;
		vertical_n_dir = m_sagittal_n_dir;
		break;
	case CORONAL_SLICE:
		new_axis_center = m_coronal_topleft_pt + (x * m_coronal_w_dir + y * m_coronal_h_dir) * m_coronal_size;
		horizontal_n_dir = m_axial_n_dir;
		vertical_n_dir = m_sagittal_n_dir;
		break;
	case SAGITTAL_SLICE:
		new_axis_center = m_sagittal_topleft_pt + (x * m_sagittal_w_dir + y * m_sagittal_h_dir) * m_sagittal_size;
		horizontal_n_dir = m_axial_n_dir;
		vertical_n_dir = m_coronal_n_dir;
	}

	d_vec = new_axis_center - m_axis_center_pt;
	horizontal_d_vec = _get_projection(d_vec, horizontal_n_dir) * horizontal_n_dir;
	vertical_d_vec = _get_projection(d_vec, vertical_n_dir) * vertical_n_dir;

	// check if outside data
	new_axis_center = m_axis_center_pt;
	if (h)
		new_axis_center = new_axis_center + horizontal_d_vec;
	if (v)
		new_axis_center = new_axis_center + vertical_d_vec;
	if (_is_outside(vol_mgr, new_axis_center))
		return 0;

	m_axis_center_pt = new_axis_center;

	switch (slice_type) {
	case AXIAL_SLICE:
		if (h)
			m_coronal_topleft_pt = m_coronal_topleft_pt + horizontal_d_vec;
		if (v)
			m_sagittal_topleft_pt = m_sagittal_topleft_pt + vertical_d_vec;
		break;
	case CORONAL_SLICE:
		if (h)
			m_axial_topleft_pt = m_axial_topleft_pt + horizontal_d_vec;
		if (v)
			m_sagittal_topleft_pt = m_sagittal_topleft_pt + vertical_d_vec;
		break;
	case SAGITTAL_SLICE:
		if (h)
			m_axial_topleft_pt = m_axial_topleft_pt + horizontal_d_vec;
		if (v)
			m_coronal_topleft_pt = m_coronal_topleft_pt + vertical_d_vec;
	}

	return 1;
}

int MPRMgr::scroll_slice(SliceType slice_type, VolumeMgr* vol_mgr, float d)
{
	// move slice by wheel, move with unit
	float cur_loc, temp_loc, next_loc, spacing;
	cgip::CgipVector d_vec;
	cgip::CgipPoint new_axis_center;
	switch (slice_type) {
	case AXIAL_SLICE:
		d_vec = m_axial_n_dir;
		cur_loc = m_axis_center_pt.z();
		spacing = vol_mgr->getSpacingZ();
		break;
	case CORONAL_SLICE:
		d_vec = m_coronal_n_dir;
		cur_loc = m_axis_center_pt.y();
		spacing = vol_mgr->getSpacingY();
		break;
	case SAGITTAL_SLICE:
		d_vec = m_sagittal_n_dir;
		cur_loc = m_axis_center_pt.x();
		spacing = vol_mgr->getSpacingX();
	}

	d = _get_dist_next_slice(d, cur_loc, spacing);
	//d *= spacing;
	if (slice_type == AXIAL_SLICE)
		d *= -1;

	// move axis center
	new_axis_center = m_axis_center_pt + d * d_vec;

	// check if outside data
	if (_is_outside(vol_mgr, new_axis_center)) {
		return 0;
	}

	m_axis_center_pt = new_axis_center;
	switch (slice_type) {
	case AXIAL_SLICE:
		m_axial_topleft_pt = m_axial_topleft_pt + d * d_vec;
		break;
	case CORONAL_SLICE:
		m_coronal_topleft_pt = m_coronal_topleft_pt + d * d_vec;
		break;
	case SAGITTAL_SLICE:
		m_sagittal_topleft_pt = m_sagittal_topleft_pt + d * d_vec;
	}

	return 1;
}

int MPRMgr::align_center(SliceType slice_type, VolumeMgr* vol_mgr, float x, float y)
{
	cgip::CgipVector d_vec;
	cgip::CgipPoint new_axis_center;
	switch (slice_type) {
	case AXIAL_SLICE:
		new_axis_center = m_axial_topleft_pt + (x * m_axial_w_dir + y * m_axial_h_dir) * m_axial_size;
		break;
	case CORONAL_SLICE:
		new_axis_center = m_coronal_topleft_pt + (x * m_coronal_w_dir + y * m_coronal_h_dir) * m_coronal_size;
		break;
	case SAGITTAL_SLICE:
		new_axis_center = m_sagittal_topleft_pt + (x * m_sagittal_w_dir + y * m_sagittal_h_dir) * m_sagittal_size;
	}

	// check if outside data
	if (_is_outside(vol_mgr, new_axis_center))
		return 0;

	m_axis_center_pt = new_axis_center;
	_resetTopLeftPt();

	return 1;
}

int MPRMgr::zoom_slice(SliceType slice_type, VolumeMgr* vol_mgr, float d)
{
	float upper_bound = std::max(vol_mgr->getWidthMM(), std::max(vol_mgr->getHeightMM(), vol_mgr->getDepthMM())) * 5;
	float lower_bound = std::min(vol_mgr->getWidthMM(), std::min(vol_mgr->getHeightMM(), vol_mgr->getDepthMM())) / 10;


	switch (slice_type) {
	case AXIAL_SLICE:
		d *= m_axial_size;
		if (m_axial_size + d < lower_bound || upper_bound < m_axial_size + d)
			return 0;
		//printf("original _d: %f", d);
		m_axial_size += d;

		m_axial_w_sampling_len = m_axial_size / (m_w_sampling_cnt - 1);
		m_axial_h_sampling_len = m_axial_size / (m_h_sampling_cnt - 1);


		printf("d for moving :%f ", d);

		m_axial_topleft_pt = m_axial_topleft_pt - (d / 2) * (m_axial_w_dir + m_axial_h_dir);
		break;

	case CORONAL_SLICE:
		d *= m_coronal_size;
		if (m_coronal_size + d < lower_bound || upper_bound < m_coronal_size + d)
			return 0;

		m_coronal_size += d;
		m_coronal_w_sampling_len = m_coronal_size / (m_w_sampling_cnt - 1);
		m_coronal_h_sampling_len = m_coronal_size / (m_h_sampling_cnt - 1);
		m_coronal_topleft_pt = m_coronal_topleft_pt + (-d / 2) * (m_coronal_w_dir + m_coronal_h_dir);
		break;
	case SAGITTAL_SLICE:
		d *= m_sagittal_size;
		if (m_sagittal_size + d < lower_bound || upper_bound < m_sagittal_size + d)
			return 0;

		m_sagittal_size += d;
		m_sagittal_w_sampling_len = m_sagittal_size / (m_w_sampling_cnt - 1);
		m_sagittal_h_sampling_len = m_sagittal_size / (m_h_sampling_cnt - 1);
		m_sagittal_topleft_pt = m_sagittal_topleft_pt + (-d / 2) * (m_sagittal_w_dir + m_sagittal_h_dir);
	}

	return 1;
}

int MPRMgr::zoom_slice_ratio(SliceType slice_type, VolumeMgr* vol_mgr, float d)
{
	float move_amount = 0;

	float upper_bound = std::max(vol_mgr->getWidthMM(), std::max(vol_mgr->getHeightMM(), vol_mgr->getDepthMM())) * 5;
	float lower_bound = std::min(vol_mgr->getWidthMM(), std::min(vol_mgr->getHeightMM(), vol_mgr->getDepthMM())) / 10;

	switch (slice_type) {
	case AXIAL_SLICE:


		//d *= m_axial_size;

		move_amount = -(m_axial_size - (1 / d * (init_axial_size / m_axial_size)) * m_axial_size);

		//printf("original _d: %f", d);




		if (m_axial_size + move_amount < lower_bound || upper_bound < m_axial_size + move_amount) {
			return 0;
		}
		m_axial_size = m_axial_size + move_amount;
		m_axial_w_sampling_len = m_axial_size / (m_w_sampling_cnt - 1);
		m_axial_h_sampling_len = m_axial_size / (m_h_sampling_cnt - 1);

		m_axial_topleft_pt = m_axial_topleft_pt - (move_amount / 2) * (m_axial_w_dir + m_axial_h_dir);


		break;
	case CORONAL_SLICE:

		//d *= m_coronal_size;

		move_amount = -(m_coronal_size - (1 / d * (init_coronal_size / m_coronal_size)) * m_coronal_size);
		if (m_coronal_size + d < lower_bound || upper_bound < m_coronal_size + d)
			return 0;

		m_coronal_size = m_coronal_size + move_amount;

		//m_coronal_size += d;
		m_coronal_w_sampling_len = m_coronal_size / (m_w_sampling_cnt - 1);
		m_coronal_h_sampling_len = m_coronal_size / (m_h_sampling_cnt - 1);
		m_coronal_topleft_pt = m_coronal_topleft_pt - (move_amount / 2) * (m_coronal_w_dir + m_coronal_h_dir);
		break;
	case SAGITTAL_SLICE:

		move_amount = -(m_sagittal_size - (1 / d * (init_sagittal_size / m_sagittal_size)) * m_sagittal_size);
		if (m_sagittal_size + d < lower_bound || upper_bound < m_sagittal_size + d)
			return 0;

		m_sagittal_size = m_sagittal_size + move_amount;

		//m_sagittal_size += d;
		m_sagittal_w_sampling_len = m_sagittal_size / (m_w_sampling_cnt - 1);
		m_sagittal_h_sampling_len = m_sagittal_size / (m_h_sampling_cnt - 1);
		m_sagittal_topleft_pt = m_sagittal_topleft_pt - (move_amount / 2) * (m_sagittal_w_dir + m_sagittal_h_dir);
	}

	return 1;
}






int MPRMgr::slice_panning(SliceType slice_type, VolumeMgr* vol_mgr, float dx, float dy)
{
	cgip::CgipPoint new_topleft;
	switch (slice_type) {
	case AXIAL_SLICE:
		new_topleft = m_axial_topleft_pt + (dx * m_axial_w_dir + dy * m_axial_h_dir) * m_axial_size;
		if (vol_mgr->getWidthMM() < new_topleft.x() ||
			vol_mgr->getHeightMM() < new_topleft.y() ||
			new_topleft.x() + (m_w_sampling_cnt - 1) * m_axial_w_sampling_len < 0 ||
			new_topleft.y() + (m_h_sampling_cnt - 1) * m_axial_h_sampling_len < 0)
			return 0;

		m_axial_topleft_pt = new_topleft;
		break;
	case CORONAL_SLICE:
		new_topleft = m_coronal_topleft_pt + (dx * m_coronal_w_dir + dy * m_coronal_h_dir) * m_coronal_size;
		if (vol_mgr->getWidthMM() < new_topleft.x() ||
			vol_mgr->getDepthMM() < new_topleft.z() ||
			new_topleft.x() + (m_w_sampling_cnt - 1) * m_coronal_w_sampling_len < 0 ||
			new_topleft.z() + (m_h_sampling_cnt - 1) * m_coronal_h_sampling_len < 0)
			return 0;

		m_coronal_topleft_pt = new_topleft;
		break;
	case SAGITTAL_SLICE:
		new_topleft = m_sagittal_topleft_pt + (dx * m_sagittal_w_dir + dy * m_sagittal_h_dir) * m_sagittal_size;
		if (vol_mgr->getHeightMM() < new_topleft.y() ||
			vol_mgr->getDepthMM() < new_topleft.z() ||
			new_topleft.y() + (m_w_sampling_cnt - 1) * m_sagittal_w_sampling_len < 0 ||
			new_topleft.z() + (m_h_sampling_cnt - 1) * m_sagittal_h_sampling_len < 0) {
			return 0;
		}

		m_sagittal_topleft_pt = new_topleft;
	}

	return 1;
}

int MPRMgr::thickness(SliceType slice_type, int v) {
	switch (slice_type) {
	case AXIAL_SLICE:
		m_axial_thickness = v;
		break;
	case CORONAL_SLICE:
		m_coronal_thickness = v;
		break;
	case SAGITTAL_SLICE:
		m_sagittal_thickness = v;
		break;
	}

	return 1;
}

int MPRMgr::inverse(SliceType slice_type, bool v) {
	switch (slice_type) {
	case AXIAL_SLICE:
		m_axial_inverse = v;
		break;
	case CORONAL_SLICE:
		m_sagittal_inverse = v;
		break;
	case SAGITTAL_SLICE:
		m_coronal_inverse = v;
		break;
	}

	return 1;
}

float MPRMgr::_add_angle(float a, float b)
{
	float res = a + b;

	if (res >= 2 * PI)
		res -= 2 * PI;
	else if (res < 0)
		res += 2 * PI;

	return res;
}

int MPRMgr::rotate_slice(SliceType slice_type, float a)
{
	Eigen::Vector3f rot_center, rot_axis;
	Eigen::Affine3f rot_mat, trans_rot_mat;

	rot_center = m_axis_center_pt.toEigen();
	switch (slice_type) {
	case AXIAL_SLICE:
		rot_axis = m_axial_n_dir.toEigen();
		break;
	case CORONAL_SLICE:
		rot_axis = m_coronal_n_dir.toEigen();
		break;
	case SAGITTAL_SLICE:
		rot_axis = m_sagittal_n_dir.toEigen();
	}

	rot_mat = Eigen::AngleAxisf(a, rot_axis);
	trans_rot_mat = Eigen::Translation3f(rot_center) * rot_mat * Eigen::Translation3f(-rot_center);

	// apply rotation
	switch (slice_type) {
	case AXIAL_SLICE:
		m_angle_on_axial = _add_angle(m_angle_on_axial, a);

		m_coronal_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_coronal_topleft_pt.toEigen(1)).head<3>());
		m_coronal_w_dir = cgip::CgipVector((rot_mat * m_coronal_w_dir.toEigen(1)).head<3>());
		m_coronal_h_dir = cgip::CgipVector((rot_mat * m_coronal_h_dir.toEigen(1)).head<3>());
		m_coronal_n_dir = cgip::CgipVector((rot_mat * m_coronal_n_dir.toEigen(1)).head<3>());

		m_sagittal_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_sagittal_topleft_pt.toEigen(1)).head<3>());
		m_sagittal_w_dir = cgip::CgipVector((rot_mat * m_sagittal_w_dir.toEigen(1)).head<3>());
		m_sagittal_h_dir = cgip::CgipVector((rot_mat * m_sagittal_h_dir.toEigen(1)).head<3>());
		m_sagittal_n_dir = cgip::CgipVector((rot_mat * m_sagittal_n_dir.toEigen(1)).head<3>());
		break;
	case CORONAL_SLICE:
		m_angle_on_coronal = _add_angle(m_angle_on_coronal, a);

		m_axial_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_axial_topleft_pt.toEigen(1)).head<3>());
		m_axial_w_dir = cgip::CgipVector((rot_mat * m_axial_w_dir.toEigen(1)).head<3>());
		m_axial_h_dir = cgip::CgipVector((rot_mat * m_axial_h_dir.toEigen(1)).head<3>());
		m_axial_n_dir = cgip::CgipVector((rot_mat * m_axial_n_dir.toEigen(1)).head<3>());

		m_sagittal_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_sagittal_topleft_pt.toEigen(1)).head<3>());
		m_sagittal_w_dir = cgip::CgipVector((rot_mat * m_sagittal_w_dir.toEigen(1)).head<3>());
		m_sagittal_h_dir = cgip::CgipVector((rot_mat * m_sagittal_h_dir.toEigen(1)).head<3>());
		m_sagittal_n_dir = cgip::CgipVector((rot_mat * m_sagittal_n_dir.toEigen(1)).head<3>());
		break;
	case SAGITTAL_SLICE:
		m_angle_on_sagittal = _add_angle(m_angle_on_sagittal, a);

		m_axial_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_axial_topleft_pt.toEigen(1)).head<3>());
		m_axial_w_dir = cgip::CgipVector((rot_mat * m_axial_w_dir.toEigen(1)).head<3>());
		m_axial_h_dir = cgip::CgipVector((rot_mat * m_axial_h_dir.toEigen(1)).head<3>());
		m_axial_n_dir = cgip::CgipVector((rot_mat * m_axial_n_dir.toEigen(1)).head<3>());

		m_coronal_topleft_pt = cgip::CgipPoint((trans_rot_mat * m_coronal_topleft_pt.toEigen(1)).head<3>());
		m_coronal_w_dir = cgip::CgipVector((rot_mat * m_coronal_w_dir.toEigen(1)).head<3>());
		m_coronal_h_dir = cgip::CgipVector((rot_mat * m_coronal_h_dir.toEigen(1)).head<3>());
		m_coronal_n_dir = cgip::CgipVector((rot_mat * m_coronal_n_dir.toEigen(1)).head<3>());
	}

	return 1;
}

int MPRMgr::windowing(SliceType slice_type, float dx, float dy) {
	/*
	m_window_width += dx;
	m_window_level -= dy;

	int min_val = m_volume_mgr->getMinVal();
	int max_val = m_volume_mgr->getMaxVal();

	if (m_window_width > max_val - min_val)
		m_window_width = max_val - min_val;
	if (m_window_level < min_val)
		m_window_level = min_val;
	if (m_window_level > max_val)
		m_window_level = max_val;
	if (m_window_width < 10)
		m_window_width = 10;
	*/

	switch (slice_type) {
	case AXIAL_SLICE:
		m_axial_window_width = dx;
		m_axial_window_level = dy;
		break;
	case CORONAL_SLICE:
		m_coronal_window_width = dx;
		m_coronal_window_level = dy;
		break;
	case SAGITTAL_SLICE:
		m_sagittal_window_width = dx;
		m_sagittal_window_level = dy;
		break;
	}

	return 1;
}

std::tuple<float, float, float> MPRMgr::getSliceLineInfo(SliceType slice_type)
{
	float axis_x, axis_y, angle;
	cgip::CgipVector target;
	switch (slice_type) {
	case AXIAL_SLICE:
		target = m_axis_center_pt - m_axial_topleft_pt;
		axis_x = _get_projection(target, m_axial_w_dir) / m_axial_size;
		axis_y = _get_projection(target, m_axial_h_dir) / m_axial_size;
		angle = m_angle_on_axial;
		break;
	case CORONAL_SLICE:
		target = m_axis_center_pt - m_coronal_topleft_pt;
		axis_x = _get_projection(target, m_coronal_w_dir) / m_coronal_size;
		axis_y = _get_projection(target, m_coronal_h_dir) / m_coronal_size;
		angle = m_angle_on_coronal;
		break;
	case SAGITTAL_SLICE:
		target = m_axis_center_pt - m_sagittal_topleft_pt;
		axis_x = _get_projection(target, m_sagittal_w_dir) / m_sagittal_size;
		axis_y = _get_projection(target, m_sagittal_h_dir) / m_sagittal_size;
		angle = m_angle_on_sagittal;
	}

	return { axis_x , axis_y, angle };
}

std::tuple<float, float> MPRMgr::getThicknessInfo(SliceType slice_type) {
	float thickness_h, thickness_v;
	switch (slice_type) {
	case AXIAL_SLICE:
		thickness_h = m_coronal_thickness / m_axial_size;
		thickness_v = m_sagittal_thickness / m_axial_size;
		break;
	case CORONAL_SLICE:
		thickness_h = m_axial_thickness / m_coronal_size;
		thickness_v = m_sagittal_thickness / m_coronal_size;
		break;
	case SAGITTAL_SLICE:
		thickness_h = m_axial_thickness / m_sagittal_size;
		thickness_v = m_coronal_thickness / m_sagittal_size;
	}
	return { thickness_h, thickness_v };
}

cgip::CgipPlane MPRMgr::_setMPR(cgip::CgipPoint& topleft_p,
	cgip::CgipVector w_dir,
	cgip::CgipVector h_dir,
	cgip::CgipVector n_dir,
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

	m_gl_mgr->getMPRShader()->use();

	m_gl_mgr->getMPRShader()->setMat4("mvMatrix", m_camera);
	m_gl_mgr->getMPRShader()->setMat4("projMatrix", m_proj);

	m_gl_mgr->getMPRShader()->setVec3("P_screen", glm::vec3(topleft_p.x(), topleft_p.y(), topleft_p.z()));
	m_gl_mgr->getMPRShader()->setVec3("w_dir", glm::vec3(w_dir.x(), w_dir.y(), w_dir.z()));
	m_gl_mgr->getMPRShader()->setVec3("h_dir", glm::vec3(h_dir.x(), h_dir.y(), h_dir.z()));
	m_gl_mgr->getMPRShader()->setVec3("n_dir", glm::vec3(n_dir.x(), n_dir.y(), n_dir.z()));

	m_gl_mgr->getMPRShader()->setFloat("m_spacingX", m_gl_mgr->getSpacingX());
	m_gl_mgr->getMPRShader()->setFloat("m_spacingY", m_gl_mgr->getSpacingY());
	m_gl_mgr->getMPRShader()->setFloat("m_spacingZ", m_gl_mgr->getSpacingZ());

	m_gl_mgr->getMPRShader()->setInt("N_x", m_gl_mgr->getWidth());
	m_gl_mgr->getMPRShader()->setInt("N_y", m_gl_mgr->getHeight());
	m_gl_mgr->getMPRShader()->setInt("N_z", m_gl_mgr->getDepth());

	m_gl_mgr->getMPRShader()->setInt("w_cnt", w_cnt);
	m_gl_mgr->getMPRShader()->setInt("h_cnt", h_cnt);
	m_gl_mgr->getMPRShader()->setInt("n_cnt", n_cnt);

	m_gl_mgr->getMPRShader()->setFloat("w_len", w_len);
	m_gl_mgr->getMPRShader()->setFloat("h_len", h_len);
	m_gl_mgr->getMPRShader()->setFloat("n_len", n_len);

	m_gl_mgr->getMPRShader()->setFloat("window_level", window_level - m_gl_mgr->getMinVal());
	m_gl_mgr->getMPRShader()->setFloat("window_width", window_width);

	m_gl_mgr->getMPRShader()->setBool("thickness", is_thickness);
	m_gl_mgr->getMPRShader()->setInt("thickness_mode", m_thickness_mode);
	m_gl_mgr->getMPRShader()->setBool("inverse", is_inverse);

	m_gl_mgr->setMaskShaderParam(MPR);

	m_gl_mgr->getMPRShader()->setInt("volume_data", 0);
	m_gl_mgr->getMPRShader()->setInt("mask_data", 1);

	m_gl_mgr->getMPRShader()->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_gl_mgr->getDataTextureId());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_gl_mgr->getMaskTextureId());

	glBindVertexArray(m_gl_mgr->getVAOId());

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, w_cnt, h_cnt);
	glDrawArrays(GL_TRIANGLES, 0, 6 * w_cnt * h_cnt);
	glBindTexture(GL_TEXTURE_2D, 0);

	int bytesPerPixel = 3;

	int strideSize = (w_cnt * (4 - bytesPerPixel)) % 4;

	int bytesPerLine = w_cnt * 3 + strideSize;

	int size = bytesPerLine * h_cnt;

	GLubyte* imageData = new GLubyte[size];

	glFinish();
	glReadPixels(0, 0, w_cnt, h_cnt, GL_RGB, GL_UNSIGNED_BYTE, imageData);

	//for debugging
	//QImage image(imageData, w_cnt, h_cnt, bytesPerLine, QImage::Format::Format_RGB888);
	//bool result = image.save("test_mpr_image.png", "PNG");
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

float MPRMgr::_get_projection(cgip::CgipVector a, cgip::CgipVector b)
{
	float result, pr;
	pr = a.dot(b);
	result = (pr * b).len();
	if (pr < 0)
		result *= -1;

	return result;
}

float MPRMgr::_get_dist_next_slice(float d, float cur_loc, float spacing) {
	float temp_loc = 0.0;
	float next_loc = 0.0;
	cgip::CgipPoint new_axis_center;

	temp_loc = cur_loc / spacing;
	next_loc = floor(temp_loc);

	// d == 1 if scroll down
	if (d > 0) {
		if (temp_loc - next_loc < EPSILON) {
			next_loc += d;
		}
		else if (1 + next_loc - temp_loc < EPSILON) {
			next_loc += d + 1;
		}
		else {
			next_loc += d;
		}
	}
	else { // d == -1 if scroll up
		if (temp_loc - next_loc < EPSILON) {
			next_loc += d;
		}
	}


	next_loc *= spacing;
	d = next_loc - cur_loc;

	return d;
}

cgip::CgipPoint MPRMgr::getCoordInfo(SliceType slice_type, float x, float y)
{
	cgip::CgipPoint pt;
	switch (slice_type) {
	case AXIAL_SLICE:
		pt = m_axial_topleft_pt + (x * m_axial_w_dir + y * m_axial_h_dir) * m_axial_w_sampling_len;
		break;
	case CORONAL_SLICE:
		pt = m_coronal_topleft_pt + (x * m_coronal_w_dir + y * m_coronal_h_dir) * m_coronal_w_sampling_len;
		break;
	case SAGITTAL_SLICE:
		pt = m_sagittal_topleft_pt + (x * m_sagittal_w_dir + y * m_sagittal_h_dir) * m_sagittal_w_sampling_len;
	}

	float x_num, y_num, z_num;
	x_num = pt.x(); // (m_volume_mgr->getSpacingX());
	y_num = pt.y(); // (m_volume_mgr->getSpacingY());
	z_num = pt.z(); // (m_volume_mgr->getSpacingZ());

	return cgip::CgipPoint(x_num, y_num, z_num + m_slice_num_offset);
}

std::tuple<int, int, int> MPRMgr::getSliceNumInfo(SliceType slice_type)
{
	float x_num, y_num, z_num;
	switch (slice_type) {
	case AXIAL_SLICE:
		z_num = m_axis_center_pt.z() / (m_gl_mgr->getSpacingZ()) + m_slice_num_offset;
		return { 0, m_gl_mgr->getDepth(), std::round(z_num) };
	case CORONAL_SLICE:
		y_num = m_axis_center_pt.y() / (m_gl_mgr->getSpacingY());
		return { 0, m_gl_mgr->getHeight(), std::round(y_num) };
	case SAGITTAL_SLICE:
		x_num = m_axis_center_pt.x() / (m_gl_mgr->getSpacingX());
		return { 0, m_gl_mgr->getWidth(), std::round(x_num) };
	}

	//float x_num, y_num, z_num;
	//x_num = pt.x()/(m_gl_mgr->getSpacingX());
	//y_num = pt.y()/(m_gl_mgr->getSpacingY());
	//z_num = pt.z()/(m_gl_mgr->getSpacingZ());

	//return cgip::CgipPoint(x_num, y_num, z_num + m_slice_num_offset);
}

std::tuple<int, int> MPRMgr::getPanningInfo(SliceType slice_type)
{
	float x_num, y_num;
	switch (slice_type) {
	case AXIAL_SLICE:
		x_num = m_axis_center_pt.x() / (m_gl_mgr->getSpacingX());
		y_num = m_gl_mgr->getHeight() - m_axis_center_pt.y() / (m_gl_mgr->getSpacingY());
		return { x_num, y_num };
	case CORONAL_SLICE:
		x_num = m_gl_mgr->getWidth() - m_axis_center_pt.x() / (m_gl_mgr->getSpacingX());
		y_num = m_axis_center_pt.z() / (m_gl_mgr->getSpacingZ());
		return { x_num, y_num };
	case SAGITTAL_SLICE:
		x_num = m_axis_center_pt.y() / (m_gl_mgr->getSpacingY());
		y_num = m_axis_center_pt.z() / (m_gl_mgr->getSpacingZ());
		return { x_num, y_num };
	}

	//float x_num, y_num, z_num;
	//x_num = pt.x()/(m_gl_mgr->getSpacingX());
	//y_num = pt.y()/(m_gl_mgr->getSpacingY());
	//z_num = pt.z()/(m_gl_mgr->getSpacingZ());

	//return cgip::CgipPoint(x_num, y_num, z_num + m_slice_num_offset);
}

void MPRMgr::getViewingPlane(
	cgip::CgipPoint& topleft_p,
	cgip::CgipVector w_dir,
	cgip::CgipVector h_dir,
	int w_cnt,
	int h_cnt,
	float w_len,
	float h_len)
{
	*m_raw = std::move(cgip::CgipPlane(w_cnt, h_cnt));
	w_dir.normalize();
	h_dir.normalize();

#pragma omp parallel for
	for (int xy = 0; xy < w_cnt * h_cnt; xy++) {
		int x = xy / w_cnt;
		int y = xy % w_cnt;
		cgip::CgipPoint p_cur = topleft_p + x * w_len * w_dir + y * h_len * h_dir;
		CgipFloat xx = p_cur.x() / m_gl_mgr->getSpacingX();
		CgipFloat yy = p_cur.y() / m_gl_mgr->getSpacingY();
		CgipFloat zz = p_cur.z() / m_gl_mgr->getSpacingZ();

		CgipShort val = m_gl_mgr->m_volume_mgr->getVoxelValue(xx, yy, zz);
		m_raw->setPixelValue(x, y, val);
	}
}

std::tuple<cgip::CgipPoint, cgip::CgipPoint> MPRMgr::_find_intersection_point(cgip::CgipPoint center, cgip::CgipVector dir) {
	float W = m_gl_mgr->getWidth() * m_gl_mgr->getSpacingX();
	float H = m_gl_mgr->getHeight() * m_gl_mgr->getSpacingY();
	float D = m_gl_mgr->getDepth() * m_gl_mgr->getSpacingZ();
	float t, temp_x, temp_y, temp_z;
	float t2, temp_x2, temp_y2, temp_z2;

	int type;

	dir.normalize();
	if (dir.x() >= dir.y() && dir.x() >= dir.z()) {
		type = 0;
	}
	else if (dir.y() > dir.x() && dir.y() > dir.z()) {
		type = 2;
	}
	else if (dir.z() > dir.x() && dir.z() > dir.y()) {
		type = 1;
	}

	int num = 0;
	cgip::CgipPoint first, second;
	switch (type) {
	case 0:
		t = (W - center.x()) / dir.x();
		temp_y = center.y() + dir.y() * t;
		temp_z = center.z() + dir.z() * t;
		if (temp_y >= 0 && temp_y <= H && temp_z >= 0 && temp_z <= D) {
			first = cgip::CgipPoint(W, temp_y, temp_z);

			t2 = (-center.x()) / dir.x();
			temp_y2 = center.y() + dir.y() * t2;
			temp_z2 = center.z() + dir.z() * t2;
			if (temp_y2 >= 0 && temp_y2 <= H && temp_z2 >= 0 && temp_z2 <= D) {
				second = cgip::CgipPoint(0, temp_y2, temp_z2);
			}

			if (t < t2) {
				return { first, second };
			}
			else {
				return { second, first };
			}
		}

	case 1:
		t = (D - center.z()) / dir.z();
		temp_x = center.x() + dir.x() * t;
		temp_y = center.y() + dir.y() * t;
		if (temp_x >= 0 && temp_x <= W && temp_y >= 0 && temp_y <= H) {
			first = cgip::CgipPoint(temp_x, temp_y, D);

			t2 = (-center.z()) / dir.z();
			temp_x2 = center.x() + dir.x() * t2;
			temp_y2 = center.y() + dir.y() * t2;
			if (temp_x2 >= 0 && temp_x2 <= W && temp_y2 >= 0 && temp_y2 <= H) {
				second = cgip::CgipPoint(temp_x2, temp_y2, 0);
			}

			if (t < t2) {
				return { first, second };
			}
			else {
				return { second, first };
			}
		}

	case 2:
		t = (H - center.y()) / dir.y();
		temp_x = center.x() + dir.x() * t;
		temp_z = center.z() + dir.z() * t;
		if (temp_x >= 0 && temp_x <= W && temp_z >= 0 && temp_z <= D) {
			first = cgip::CgipPoint(temp_x, W, temp_z);

			t2 = (-center.y()) / dir.y();
			temp_x2 = center.x() + dir.x() * t2;
			temp_z2 = center.z() + dir.z() * t2;
			if (temp_x2 >= 0 && temp_x2 <= W && temp_z2 >= 0 && temp_z2 <= D) {
				second = cgip::CgipPoint(temp_x2, 0, temp_z2);
			}

			if (t < t2) {
				return { first, second };
			}
			else {
				return { second, first };
			}
		}
	}
}
