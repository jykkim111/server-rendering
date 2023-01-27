#pragma once


// Qt includes
#include <QObject>

// Global includes
#include "../CGIPIO/CgipFileIO.h"
#include "../CGIPIO/CgipRawIO.h"
#include "../CGIPIO/CgipDicomIO.h"
#include "../CgipCommon/CgipVolume.h"
#include "../CgipCommon/CgipPlane.h"
#include "../CgipCommon/CgipPoint.h"
#include "../CgipCommon/CgipVector.h"
#include "glm/gtx/transform.hpp"
#include "glm/ext.hpp"

// Local includes
#include "viewermanager.h"
#include "global.h"
#include "Mgr.h"

class VIEWERMANAGER_DLL VolumeMgr : public Mgr
{
	Q_OBJECT

public:
	VolumeMgr();
	~VolumeMgr();

	/**
	 *	Name:	readVolume
	 *	Params:
	 *		- files:	std::vector<std::string>
	 *			paths of image files
	 *		- type:		RemoteTreeType
	 *			prj code and modality information
	 *	Desc:	read volume data from image files
	 */
	void readVolume(std::vector<std::string> files);
	void readVolume(std::string vol_file, std::string meta_file);

	// TODO:: modify this
	void readPlane(std::string file);

	// Access the raw pointer
	inline cgip::CgipVolume* getRawPtr() { return m_volume; };
	inline cgip::CgipVolume* getRawPtr_R() { return m_volume_r; };
	inline cgip::CgipVolume* getRawPtr_G() { return m_volume_g; };
	inline cgip::CgipVolume* getRawPtr_B() { return m_volume_b; };
	inline CgipShort getVoxelValue(int x, int y, int z) { return m_volume->getVoxelValue(x, y, z, m_min_val); };

	cgip::CgipPlane getViewingPlane(cgip::CgipPoint& topleft_p, cgip::CgipVector w_dir, cgip::CgipVector h_dir, CgipInt w_cnt, CgipInt h_cnt, CgipFloat w_len, CgipFloat h_len);
	cgip::CgipPlane getViewingPlane(cgip::CgipPoint& topleft_p, cgip::CgipVector w_dir, cgip::CgipVector h_dir, cgip::CgipVector n_dir, CgipInt w_cnt, CgipInt h_cnt, CgipInt d_cnt, CgipFloat w_len, CgipFloat h_len, CgipFloat d_len);

	inline CgipFloat getWidthMM() { return (m_volume->getWidth() - 1) * m_volume->getSpacingX(); }
	inline CgipFloat getHeightMM() { return (m_volume->getHeight() - 1) * m_volume->getSpacingY(); }
	inline CgipFloat getDepthMM() { return (m_volume->getDepth() - 1) * m_volume->getSpacingZ(); }
	inline CgipFloat getSpacingX() { return m_volume->getSpacingX(); }
	inline CgipFloat getSpacingY() { return m_volume->getSpacingY(); }
	inline CgipFloat getSpacingZ() { return m_volume->getSpacingZ(); }
	inline void setWidth(int width) { m_width = width; }
	inline int getWidth() { return m_width; }
	inline void setHeight(int height) { m_height = height; }
	inline int getHeight() { return m_height; }
	inline void setDepth(int depth) { m_depth = depth; }
	inline CgipInt getDepth() { return m_depth; }
	inline CgipShort getMinVal() { return m_min_val; }
	inline CgipShort getMaxVal() { return m_max_val; }
	inline CgipInt getDefaultWindowCenter() { return m_default_window_center; }
	inline CgipInt getDefaultWindowWidth() { return m_default_window_width; }
	inline CgipInt getChannel() { return m_channel; }
	inline CgipFloat getRescaleIntercept() { return m_rescale_intercept; }
	inline CgipFloat getRescaleSlope() { return m_rescale_slope; }

	inline void setModality(std::string modality) { m_modality = modality; }
	inline std::string getModality() { return m_modality; }
	inline void setCaseId(std::string case_id) { m_case_id = case_id; }
	inline std::string getCaseId() { return m_case_id; }
	inline void setCaseCode(std::string code) { m_case_code = code; }
	inline std::string getCaseCode() { return m_case_code; }
	inline void setCategory(std::string category) { m_category = category; }
	inline std::string getCategory() { return m_category; }
	inline void setStudyUid(std::string uid) { m_study_uid = uid; }
	inline std::string getStudyUid() { return m_study_uid; }
	inline void setSeriesUid(std::string uid) { m_series_uid = uid; }
	inline std::string getSeriesUid() { return m_series_uid; }
	inline void setUidList(std::vector<std::string> uid_list) { m_uid_list = uid_list; }
	inline std::vector<std::string> getUidList() { return m_uid_list; }

	VolumePacket getVolumePacket();

	glm::mat4 getInitRotationMat() { return m_init_rotation; }
	glm::mat4 getInitShearMat() { return m_init_shear; }

	int getPAxis() { return m_pAxis; }



private:
	cgip::CgipFileIO* m_io;
	cgip::CgipVolume* m_volume; // used for grayscale
	cgip::CgipVolume* m_volume_r;
	cgip::CgipVolume* m_volume_g;
	cgip::CgipVolume* m_volume_b;

	// Volume info
	cgip::VolumeInfo m_vol_info;
	int m_width=0, m_height=0, m_depth=0;
	short m_min_val, m_max_val;
	int m_default_window_center=0, m_default_window_width=0;
	int m_channel;
	float m_rescale_intercept, m_rescale_slope;
	glm::vec3 m_ptHor, m_ptVer, m_vtFirst, m_vtSecond, m_vtLast;
	glm::mat4 m_init_rotation, m_init_shear;
	int m_pAxis;

	// Dicom series info
	std::string m_modality;
	std::string m_case_id;
	std::string m_case_code;
	std::string m_category;
	std::string m_study_uid;
	std::string m_series_uid;
	std::string m_patient_position;
	std::vector<std::string> m_uid_list;


/* ========================================================================== */
/* 							   Private Functions							  */
/* ========================================================================== */
	void _readVolume(std::vector<std::string> files);
	void _readPlane(std::string file);
	void _setValue();
	void _setCoordinateSystem();
	int _getPrincipalAxisWithSign(glm::vec3, glm::vec3, int&, bool&);
	int _getMaxAbsValueIndex(glm::vec3);
	int _getMaxAbsValueIndex(glm::vec3, glm::vec3);
};
