#include "VolumeMgr.h"
#include "../CgipCommon/cgip_types.h"
#include "../CgipException/CgipException.h"

#include <iostream>

VolumeMgr::VolumeMgr() {
	m_io = nullptr;

	m_volume = nullptr;
	m_volume_r = nullptr;
	m_volume_g = nullptr;
	m_volume_b = nullptr;
}

VolumeMgr::~VolumeMgr() {
	SAFE_DELETE_OBJECT(m_io);
	SAFE_DELETE_OBJECT(m_volume);
	SAFE_DELETE_OBJECT(m_volume_r);
	SAFE_DELETE_OBJECT(m_volume_g);
	SAFE_DELETE_OBJECT(m_volume_b);
}

/**
 *	Name:		readVolume
 *	Created:	2020-10-03
 *	Modified:	2020-12-21
 *	Author:		Sanguk Park
 *	Params:
 *		- files:	std::vector<std::string>
 *			paths of image files
 *		- type:		RemoteTreeType
 *			prj code and modality information
 *	Desc:		read volume data from image files
 */
void VolumeMgr::readVolume(std::vector<std::string> files) {

	_readVolume(files);
}

void VolumeMgr::readVolume(std::string vol_file, std::string meta_file) {

	m_io = cgip::getIO("raw");
	((cgip::CgipRawIO*)m_io)->readVolume(vol_file, meta_file);
	_setValue();
}

void VolumeMgr::readPlane(std::string file) {

	_readPlane(file);
}

VolumePacket VolumeMgr::getVolumePacket() {
	VolumePacket volume_packet;

	volume_packet.ImageRescaleIntercept = m_rescale_intercept;
	volume_packet.ImageRescaleSlope = m_rescale_slope;
	volume_packet.PixelRepresentation = m_width;
	volume_packet.PhotometricInterpretation = "";

	if (m_default_window_width == 0) {
		m_default_window_center = (m_min_val + m_max_val) / 2;
		m_default_window_width = m_max_val - m_min_val;
		if (m_default_window_width < 10)
			m_default_window_width = 10;
	}
	volume_packet.ImageWindowWidth = m_default_window_width;
	volume_packet.ImageWindowCenter = m_default_window_center;

	volume_packet.ImageCols = m_height;
	volume_packet.ImageRows = m_width;
	volume_packet.PixelSpacingCol = m_volume->getSpacingY();
	volume_packet.PixelSpacingRow = m_volume->getSpacingX();
	volume_packet.BitsStored = 12;
	volume_packet.BitsAllocated = 16;
	volume_packet.SliceThickness = m_volume->getSpacingZ();
	volume_packet.MaxVal = m_max_val;
	volume_packet.MinVal = m_min_val;

	return volume_packet;
}

/**
	 *	Name:		getViewingPlane
	 *	Created:	2020-06-30
	 *	Modified:	2021-01-22
	 *  Author:		Sanguk Park, Jusang Lee
	 *	Params:
	 *		- topleft_p:			cgip::CgipPoint&
	 *			3d point in volume of top left point the plane
	 *		- w_dir:				cgip::CgipVector
	 *			width direction vector(x dir on the plane)
	 *		- h_dir:				cgip::CgipVector
	 *			height direction vector(y dir on the plane)
	 *		- w_cnt:				int
	 *			sampling count on width direction(x dir)
	 *		- h_cnt:				int
	 *			sampling count on height direction(x dir)
	 *		- w_len:				float
	 *			width length of the plane
	 *		- h_len:				float
	 *			height length of the plane
	 *	Desc:		
	 *		Get 2-dimensional projected plane from 3-dimensional volume data
	 */
cgip::CgipPlane
VolumeMgr::getViewingPlane(
	cgip::CgipPoint& topleft_p, 
	cgip::CgipVector w_dir, 
	cgip::CgipVector h_dir, 
	int w_cnt, 
	int h_cnt,
	float w_len, 
	float h_len)
{
	cgip::CgipPlane plane(w_cnt, h_cnt, m_channel);

#pragma omp parallel for
	for (int xy = 0; xy < w_cnt * h_cnt; xy++) {
		int x = xy / w_cnt;
		int y = xy % w_cnt;
		cgip::CgipPoint p_cur = topleft_p + x * w_len * w_dir + y * h_len * h_dir;
		CgipFloat xx = p_cur.x() / m_volume->getSpacingX();
		CgipFloat yy = p_cur.y() / m_volume->getSpacingY();
		CgipFloat zz = p_cur.z() / m_volume->getSpacingZ();

		if (m_channel == 1) {
			CgipShort val = m_volume->getVoxelValue(xx, yy, zz,	m_min_val);
			plane.setPixelValue(x, y, val);
		}
		else if (m_vol_info.photometry == "YUV") {
			short luma = m_volume_r->getVoxelValue(xx, yy, zz, 0);
			short cb = m_volume_g->getVoxelValue(xx, yy, zz, 128);
			short cr = m_volume_b->getVoxelValue(xx, yy, zz, 128);

			short val_r = luma							+ (cr - 128) * 1.40200;
			short val_g = luma + (cb - 128) * -0.34414	+ (cr - 128) * -0.71414;
			short val_b = luma + (cb - 128) * 1.77200;

			plane.setPixelValue(x, y, 0, val_r);
			plane.setPixelValue(x, y, 1, val_g);
			plane.setPixelValue(x, y, 2, val_b);
		}
		else if (m_channel == 3) {
			short val_r = m_volume_r->getVoxelValue(xx, yy, zz, m_min_val);
			short val_g = m_volume_g->getVoxelValue(xx, yy, zz, m_min_val);
			short val_b = m_volume_b->getVoxelValue(xx, yy, zz, m_min_val);
			plane.setPixelValue(x, y, 0, val_r);
			plane.setPixelValue(x, y, 1, val_g);
			plane.setPixelValue(x, y, 2, val_b);
		}
	}

	return std::move(plane);
}

cgip::CgipPlane
VolumeMgr::getViewingPlane(
	cgip::CgipPoint& topleft_p,
	cgip::CgipVector w_dir,
	cgip::CgipVector h_dir,
	cgip::CgipVector n_dir,
	int w_cnt,
	int h_cnt,
	int n_cnt,
	float w_len,
	float h_len,
	float n_len) {
	cgip::CgipPlane result(w_cnt, h_cnt, m_channel);

	std::vector<float> plane;
	plane.insert(plane.end(), w_cnt * h_cnt * m_channel, 0);

	int num = 1;
	for (int i = -n_cnt; i < n_cnt; i++) {
		cgip::CgipPoint new_topleft_p = topleft_p + i * n_len * n_dir;

		cgip::CgipPlane temp_plane = getViewingPlane(new_topleft_p, w_dir, h_dir, w_cnt, h_cnt, w_len, h_len);

#pragma omp parallel for
		for (int xy = 0; xy < temp_plane.getWidth() * temp_plane.getHeight(); xy++) {
			int x = xy / temp_plane.getWidth();
			int y = xy % temp_plane.getWidth();

			for (int c = 0; c < temp_plane.getChannel(); c++) {
				plane[temp_plane.getChannel() * (y * temp_plane.getWidth() + x) + c] += (temp_plane.getPixelValue(x, y, c) -
					plane[temp_plane.getChannel() * (y * temp_plane.getWidth() + x) + c]) / num;
			}
		}
		num++;
	}
#pragma omp parallel for
	for (int xy = 0; xy < w_cnt * h_cnt; xy++) {
		int x = xy / w_cnt;
		int y = xy % w_cnt;

		for (int c = 0; c < m_channel; c++) {
			result.setPixelValue(x, y, c, plane[m_channel * (y * w_cnt + x) + c]);
		}
	}

	return std::move(result);
}

/* ========================================================================== */
/* 							   Private Functions							  */
/* ========================================================================== */
void VolumeMgr::_readVolume(std::vector<std::string> files) {

	// Logger
	std::string str_file_list = "";
	for (auto file : files) str_file_list = str_file_list + '\t' + file + '\n';
	//m_logger_dev->debug("Trying to load volume.\n {}", str_file_list);

	// Todo:: Remove, already checked in viewer.cpp
	std::string ext = cgip::checkFilesUnitary(files);

	//m_logger_dev->debug("Opening '{}' files\n", ext);

	try {
		m_io = cgip::getIO(ext.c_str());

		// Set volume dimension for raw array
		if (m_io->getExt() == "raw") {
			cgip::VolumeInfo vol_info = m_io->getVolumeInfo();
			vol_info.width = m_width;
			vol_info.height = m_height;
			vol_info.depth = m_depth;

			m_io->setVolumeInfo(vol_info);
		}

		m_io->readVolume(files);
	}
	catch (cgip::CgipException e) {
		//m_logger_dev->error(e.what());
		return;
	}
	 
	// Get uid lists for dicom images
	if (m_io->getExt() == "dcm") {
		m_uid_list = ((cgip::CgipDicomIO*)m_io)->getUidList();
	}

	_setValue();
}

void VolumeMgr::_readPlane(std::string file) {
	std::vector<std::string> files;
	files.push_back(file);

	std::string ext = cgip::checkFilesUnitary(files);

	try {
		m_io = cgip::getIO(ext.c_str());
		m_io->readPlane(file);
	}
	catch (cgip::CgipException e) {
		//m_logger_dev->error(e.what());
		return;
	}

	_setValue();
}

void VolumeMgr::_setValue() {
	SAFE_DELETE_OBJECT(m_volume);
	SAFE_DELETE_OBJECT(m_volume_r);
	SAFE_DELETE_OBJECT(m_volume_g);
	SAFE_DELETE_OBJECT(m_volume_b);

	m_vol_info = ((cgip::CgipRawIO*)m_io)->getVolumeInfo();
	m_width = m_vol_info.width;
	m_height = m_vol_info.height;
	m_depth = m_vol_info.depth;
	m_default_window_center = m_vol_info.default_window_center;
	m_default_window_width = m_vol_info.default_window_width;
	m_modality = m_vol_info.modality;
	m_channel = m_vol_info.samples_per_pixel;
	m_series_uid = m_vol_info.series_uid;
	m_rescale_intercept = m_vol_info.rescale_intercept;
	m_rescale_slope = m_vol_info.rescale_slope;
	m_ptHor = glm::vec3(m_vol_info.m_ptHor_x, m_vol_info.m_ptHor_y, m_vol_info.m_ptHor_z);
	m_ptVer = glm::vec3(m_vol_info.m_ptVer_x, m_vol_info.m_ptVer_y, m_vol_info.m_ptVer_z);
	m_vtFirst = glm::vec3(m_vol_info.m_FirstImg_x, m_vol_info.m_FirstImg_y, m_vol_info.m_FirstImg_z);
	m_vtSecond = glm::vec3(m_vol_info.m_SecondImg_x, m_vol_info.m_SecondImg_y, m_vol_info.m_SecondImg_z);
	m_vtLast = glm::vec3(m_vol_info.m_LastImg_x, m_vol_info.m_LastImg_y, m_vol_info.m_LastImg_z);
	m_patient_position = m_vol_info.patient_position;

	if (m_channel == 1) {
		m_volume = (((cgip::CgipRawIO*)m_io)->getVolumePtr())[0];
		m_min_val = m_volume->getMinVoxelValue();
		m_max_val = m_volume->getMaxVoxelValue();
	}
	else if (m_channel == 3) {
		m_volume_r = (m_io->getVolumePtr())[0];
		m_volume_g = (m_io->getVolumePtr())[1];
		m_volume_b = (m_io->getVolumePtr())[2];
		m_min_val = fmin(m_volume_r->getMinVoxelValue(), fmin(m_volume_g->getMinVoxelValue(), m_volume_b->getMinVoxelValue()));
		m_max_val = fmax(m_volume_r->getMaxVoxelValue(), fmax(m_volume_g->getMaxVoxelValue(), m_volume_b->getMaxVoxelValue()));

		// blend RGB to create graysacle
		m_volume = new cgip::CgipVolume(m_vol_info.width, m_vol_info.height, m_vol_info.depth);
		m_volume->setSpacingX(m_vol_info.spacing_x);
		m_volume->setSpacingY(m_vol_info.spacing_y);
		m_volume->setSpacingZ(m_vol_info.spacing_z);
#pragma omp parallel for
		for (int i = 0; i < m_vol_info.width * m_vol_info.height * m_vol_info.depth; i++) {
			int z = i / (m_vol_info.width * m_vol_info.height);
			int y = (i % (m_vol_info.width * m_vol_info.height)) / m_vol_info.width;
			int x = (i % (m_vol_info.width * m_vol_info.height)) % m_vol_info.width;
			int val_r = m_volume_r->getVoxelValue(x, y, z, m_min_val);
			int val_g = m_volume_g->getVoxelValue(x, y, z, m_min_val);
			int val_b = m_volume_b->getVoxelValue(x, y, z, m_min_val);
			int val = (int)(0.299 * val_r + 0.587 * val_g + 0.114 * val_b);
			m_volume->setVoxelValue(x, y, z, val);
		}
	}

	_setCoordinateSystem();

	SAFE_DELETE_OBJECT(m_io);
}

void VolumeMgr::_setCoordinateSystem() {
	int p_axis;
	bool bSign;
	_getPrincipalAxisWithSign(m_ptHor, m_ptVer, p_axis, bSign);


	glm::vec3 vtGantryAxisX = glm::vec3(m_ptHor.x, m_ptHor.y, m_ptHor.z);
	if (glm::length(vtGantryAxisX) == 0) {
		vtGantryAxisX[0] = 1;
	}
	vtGantryAxisX = glm::normalize(m_ptHor);


	glm::vec3 vtGantryAxisY = glm::vec3(m_ptVer.x, m_ptVer.y, m_ptVer.z);
	if (glm::length(vtGantryAxisY) == 0) {
		vtGantryAxisY[1] = 1;
	}
	vtGantryAxisY = glm::normalize(vtGantryAxisY);

	glm::vec3 vtGantryAxisZ = glm::cross(vtGantryAxisX, vtGantryAxisY);
	vtGantryAxisZ = glm::normalize(vtGantryAxisZ);

	glm::vec3 vtPrincipalAxis = m_vtFirst - m_vtLast;
	vtPrincipalAxis = glm::normalize(vtPrincipalAxis);
	m_pAxis = p_axis;
	std::cout << "p_axis1: " << glm::to_string(vtPrincipalAxis) << std::endl;
	std::cout << "p_axis2: " << p_axis << std::endl;
	std::cout << "bSign: " << bSign << std::endl;

	
	if (p_axis == 2) {
		if (!bSign) {
			vtPrincipalAxis = glm::vec3(0, 0, 1);
			vtGantryAxisY.z = -vtGantryAxisY.z;
			vtGantryAxisZ.y = -vtGantryAxisZ.y;
		}

	}
	else if (p_axis == 1) {
		if (!bSign) {
			vtGantryAxisY.y = -vtGantryAxisY.y;
			vtGantryAxisY.z = -vtGantryAxisY.z;
		}

	}
	else {
		if (bSign) {
			vtGantryAxisX.x = vtGantryAxisX.x;
			vtGantryAxisX.y = vtGantryAxisX.y;
			vtGantryAxisX.z = vtGantryAxisX.z;
		}

	}
	


	glm::mat4 mtGantryCenter = glm::translate(glm::mat4(1), glm::vec3(-m_width * m_volume->getSpacingX() / 2.0, -m_height * m_volume->getSpacingY() / 2.0, -m_depth * m_volume->getSpacingZ() / 2.0));
	glm::mat4 mtGantryInverseCenter = glm::translate(glm::mat4(1), glm::vec3(m_width * m_volume->getSpacingX() / 2.0, m_height * m_volume->getSpacingY() / 2.0, m_depth * m_volume->getSpacingZ() / 2.0));
	glm::mat4 mtGCStoOrthoYS;
	if (bSign) {
		mtGCStoOrthoYS = glm::mat4(vtGantryAxisX.x, vtGantryAxisX.y, -vtGantryAxisX.z, 0.0,
			vtGantryAxisY.x, vtGantryAxisY.y, -vtGantryAxisY.z, 0.0,
			-vtGantryAxisZ.x, -vtGantryAxisZ.y, vtGantryAxisZ.z, 0.0,
			0.0, 0.0, 0.0, 1.0);
	}
	else {
		mtGCStoOrthoYS = glm::mat4(vtGantryAxisX.x, vtGantryAxisX.y, vtGantryAxisX.z, 0.0,
			vtGantryAxisY.x, vtGantryAxisY.y, vtGantryAxisY.z, 0.0,
			vtGantryAxisZ.x, vtGantryAxisZ.y, vtGantryAxisZ.z, 0.0,
			0.0, 0.0, 0.0, 1.0);
	}



	//std::cout << "GCStoOrthoYS: " << glm::to_string(mtGCStoOrthoYS) << std::endl;
	glm::mat4 mtGantryRotate = glm::inverse(mtGCStoOrthoYS);
	//std::cout << "rotation matrix: " << glm::to_string(mtGantryRotate) << std::endl;

	float dfGantrySh_x = 0.0, dfGantrySh_y = 0.0;

	// mtGCStoOrthoYS.TransformVect3D(vtPrincipalAxis.x, vtPrincipalAxis.y, vtPrincipalAxis.z, &dfx, &dfy, &dfz);
	glm::vec3 df = glm::vec3(mtGCStoOrthoYS * glm::vec4(vtPrincipalAxis, 0.f));
	std::cout << "df: " << glm::to_string(df) << std::endl;
	if (df.z != 0.) {
		dfGantrySh_x = df.x / df.z;
		dfGantrySh_y = df.y / df.z;
	}


	glm::mat4 mtGantryShear;
	if (bSign) {
		mtGantryShear = glm::mat4(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);

	}
	else {
		mtGantryShear = glm::mat4(1, 0, dfGantrySh_x, 0,
			0, 1, 0, 0,
			0, dfGantrySh_y, 1, 0,
			0, 0, 0, 1);
	}

	//mtGantryShear = glm::inverse(mtGantryShear);
	std::cout << "shear matrix : " << glm::to_string(mtGantryShear) << std::endl;
	//float m_dfRatioZ = 0.0; // to be initialized
	//float dfGantryRatio = m_dfRatioZ / (dfGantrySh_x * dfGantrySh_x + dfGantrySh_y * dfGantrySh_y + 1);
	//glm::mat4 mtGantryScale = glm::scale(glm::mat4(1), glm::vec3(1, 1, dfGantryRatio));

	// glm::mat4 transform = mtGantryInverseCenter * mtGantryRotate * mtGantryShear * mtGantryScale * mtGantryCenter;
	//glm::mat4 transform = mtGantryInverseCenter * mtGantryShear * mtGantryRotate  * mtGantryCenter;
	//glm::mat4 inverse_transform = glm::inverse(transform);

	//glm::mat4 transform = mtGantryInverseCenter * mtGCStoOrthoYS * mtGantryShear * mtGantryCenter;
	glm::mat4 transform = mtGantryInverseCenter * mtGantryShear * mtGantryCenter;
	m_init_rotation = mtGantryRotate;
	m_init_shear = mtGantryShear;

	//std::cout << "x_spacing: " << m_volume->getSpacingX() << std::endl;
	//std::cout << "y_spacing: " << m_volume->getSpacingY() << std::endl;
	//std::cout << "z_spacing: " << m_volume->getSpacingZ() << std::endl;


	cgip::CgipVolume* new_volume = new cgip::CgipVolume(m_width, m_height, m_depth);


#pragma omp parallel for
	for (int z = 0; z < m_volume->getDepth(); z++) {
		for (int y = 0; y < m_volume->getHeight(); y++) {
			for (int x = 0; x < m_volume->getWidth(); x++) {
				glm::vec3 position = {x * m_volume->getSpacingX(), y * m_volume->getSpacingY(), z * m_volume->getSpacingZ() };
				//glm::vec3 position = { x , y , z };
				position = glm::vec3(transform * glm::vec4(position, 1.f)) / glm::vec3(m_volume->getSpacingX(), m_volume->getSpacingY(), m_volume->getSpacingZ());
				//position = glm::vec3(transform * glm::vec4(position, 1.f));
				if (!m_volume->isOutbound(position.x, position.y, position.z)) {
					new_volume->setVoxelValue(x, y, z, m_volume->getVoxelValue(position.x, position.y, position.z));
				}
				else {
					new_volume->setVoxelValue(x, y, z, m_min_val);
				}
			}
		}
	}

#pragma omp parallel for
	for (int z = 0; z < m_volume->getDepth(); z++) {
		for (int y = 0; y < m_volume->getHeight(); y++) {
			for (int x = 0; x < m_volume->getWidth(); x++) {
				m_volume->setVoxelValue(x, y, z, new_volume->getVoxelValue(x, y, z));
			}
		}
	}

	SAFE_DELETE_OBJECT(new_volume);
}



int VolumeMgr::_getPrincipalAxisWithSign(glm::vec3 dfValue1, glm::vec3 dfValue2, int& iAxis, bool& bSign) {
	int iMax1 = _getMaxAbsValueIndex(dfValue1);
	int iMax2 = _getMaxAbsValueIndex(dfValue2);

	glm::vec3 afDir1, afDir2;

	switch (iMax1)
	{
	case 0: // (1,0,0)
	{
		if (dfValue1[iMax1] > 0)
		{
			afDir1.x = 0.;
			afDir1.y = 1.;
			afDir1.z = 0.;
		}
		else
		{
			afDir1.x = 0.;
			afDir1.y = -1.;
			afDir1.z = 0.;
		}
	}
	break;
	case 1: // (0,1,0)
	{
		if (dfValue1[iMax1] > 0)
		{
			afDir1.x = -1.;
			afDir1.y = 0.;
			afDir1.z = 0.;
		}
		else
		{
			afDir1.x = 1.;
			afDir1.y = 0.;
			afDir1.z = 0.;
		}
	}
	break;
	case 2: // (0,0,1)
	{
		if (dfValue1[iMax1] > 0)
		{
			afDir1.x = 0.;
			afDir1.y = 0.;
			afDir1.z = 1.;
		}
		else
		{
			afDir1.x = 0.;
			afDir1.y = 0.;
			afDir1.z = -1.;
		}
	}
	break;
	}

	switch (iMax2)
	{
	case 0: // (1,0,0)
	{
		if (dfValue2[iMax2] > 0)
		{
			afDir2.x = 0.;
			afDir2.y = 1.;
			afDir2.z = 0.;
		}
		else
		{
			afDir2.x = 0.;
			afDir2.y = -1.;
			afDir2.z = 0.;
		}
	}
	break;
	case 1: // (0,1,0)
	{
		if (dfValue2[iMax2] > 0)
		{
			afDir2.x = -1.;
			afDir2.y = 0.;
			afDir2.z = 0.;
		}
		else
		{
			afDir2.x = 1.;
			afDir2.y = 0.;
			afDir2.z = 0.;
		}
	}
	break;
	case 2: // (0,0,1)
	{
		if (dfValue2[iMax2] > 0)
		{
			afDir2.x = 0.;
			afDir2.y = 0.;
			afDir2.z = 1.;
		}
		else
		{
			afDir2.x = 0.;
			afDir2.y = 0.;
			afDir2.z = -1.;
		}
	}
	break;
	}

	iAxis = _getMaxAbsValueIndex(dfValue1, dfValue2);

	double u = afDir1.y * afDir2.z - afDir1.z * afDir2.y;
	double v = afDir1.z * afDir2.x - afDir1.x * afDir2.z;
	double w = afDir1.x * afDir2.y - afDir1.y * afDir2.x;

	if ((u == 1. && v == 0. && w == 0.) ||
		(u == 0. && v == -1. && w == 0.) ||
		(u == 0. && v == 0. && w == -1.))
	{
		bSign = TRUE;
	}
	else
	{
		bSign = FALSE;
	}
	return TRUE;

}

int VolumeMgr::_getMaxAbsValueIndex(glm::vec3 dfValue) {
	int iIndex = 0;

	if (fabs(dfValue[0]) > fabs(dfValue[1]))
	{
		iIndex = 0;
	}
	else
	{
		iIndex = 1;
	}

	if (fabs(dfValue[iIndex]) < fabs(dfValue[2]))
	{
		iIndex = 2;
	}

	return iIndex;
}

int VolumeMgr::_getMaxAbsValueIndex(glm::vec3 dfValue1, glm::vec3 dfValue2) {
	int iMaxIndex = 2;

	int iMax1 = _getMaxAbsValueIndex(dfValue1);
	int iMax2 = _getMaxAbsValueIndex(dfValue2);

	if ((iMax1 == 0 && iMax2 == 1) || (iMax1 == 1 && iMax2 == 0))
	{
		iMaxIndex = 2;
	}
	else if ((iMax1 == 0 && iMax2 == 2) || (iMax1 == 2 && iMax2 == 0))
	{
		iMaxIndex = 1;
	}
	else if ((iMax1 == 1 && iMax2 == 2) || (iMax1 == 2 && iMax2 == 1))
	{
		iMaxIndex = 0;
	}

	return iMaxIndex;

}