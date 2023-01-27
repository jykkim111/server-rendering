#pragma once
#include "viewermanager.h"

#include <QColor>
#include <QObject>
#include <QString>
#include <QDir>

// External library includes
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <json.hpp>
#include <jpeg8/jpeglib8.h>

// classes includes
#include "build_global_opt.h"
#include "Mgr.h"
#include "GLMgr.h"
#include "MPRMgr.h"
#include "DVRMgr.h"
#include "DVRRectMgr.h"
#include "MPRRectMgr.h"
#include "CPRMgr.h"
#include "MaskMgr.h"
#include "VolumeMgr.h"
#include "../CgipCommon/CgipVector.h"
#include "../CgipCommon/CgipPlane.h"
#include "../CgipCommon/CgipPoint.h"
#include "../CGIPIO/CgipImageIO.h"

#include "global.h"

using json = nlohmann::json;


class VIEWERMANAGER_DLL RenderingFunctionMgr : public Mgr
{
public:
	RenderingFunctionMgr();
	~RenderingFunctionMgr();

	//testing function
	void write_jpeg_file(cgip::CgipPlane*, const char*, int);

	// set & get functions
	void setMPRSamplingCnt(int w, int h, bool is_downsampling, bool skip=false);
	void setVRSamplingCnt(int c, bool is_downsampling, bool skip=false);
	void setCPRSamplingCnt(int w, int h, int mode, SliceType slice_type, bool skip = false);
	void initSamplingCnt(int mpr_w, int mpr_h, int vr);
	void setCurrentCPRMode(SliceType slice_type) { m_cpr_slice_type = slice_type; }
	SliceType getCurrentCPRMode() { return m_cpr_slice_type; }
	void setRenderMode(bool from_server) { render_from_server = from_server; }
	bool getRenderMode() { return render_from_server; }
	inline int getMPRSamplingCnt() { return m_mpr_sampling_cnt_width; }
	inline int getVRSamplingCnt() { return m_vr_sampling_cnt; }
	inline PacketUpdateInfo getPacketUpdateInfo() { return m_packet_update_info; }
	cgip::CgipPlane getMPRImage(SliceType s);
	cgip::CgipPlane getVRImage();
	cgip::CgipPlane getCPRImage();
	cgip::CgipPlane getCPRPathImage();
	json getPacketUpdateInfobyJson();
	json getVolumeInfobyJson();
	json getMPRInfobyJson(SliceType s);
	json getVRInfobyJson();
	json getMaskInfobyJson();
	json getCPRCurveInfobyJson(SliceType s);
	json getCPRPathInfobyJson();

	// function for managing actions
	void action_manager(RenderingType, ActionType, json, bool skip=false);

	// functions for Data Load
	PacketUpdateInfo loadVolume(std::string dir, bool is_leaf, int panel_width, int panel_height);
	PacketUpdateInfo loadMask(std::vector<std::string> dir);
	PacketUpdateInfo saveMask(std::string dir);

	// functions for MPR
	PacketUpdateInfo mpr_zoom(SliceType, int, float dy, bool skip = false);
	PacketUpdateInfo mpr_pan(SliceType, float dx, float dy, bool skip = false);
	PacketUpdateInfo mpr_window(SliceType, float dx, float dy, bool skip = false);
	PacketUpdateInfo mpr_center(SliceType, float x, float y);
	PacketUpdateInfo mpr_reset(SliceType);
	PacketUpdateInfo mpr_move_slice(SliceType, float d, bool skip = false);
	PacketUpdateInfo mpr_move_slice(SliceType, float x, float y, bool h, bool v, bool skip = false);
	PacketUpdateInfo mpr_oblique(SliceType, float angle, bool skip = false);
	PacketUpdateInfo mpr_thickness(SliceType, int v, bool skip=false);
	PacketUpdateInfo mpr_thickness_mode(int v, bool skip = false);
	PacketUpdateInfo mpr_Inverse(SliceType, bool v);
	PacketUpdateInfo mask_visible(int idx, bool v);
	PacketUpdateInfo mask_opacity(int v);
	PacketUpdateInfo mask_color(int idx, std::string v);
	PacketUpdateInfo mpr_raw(SliceType);


	//functions for CPR
	void cpr_init(SliceType);
	PacketUpdateInfo cpr_curve_update(SliceType, std::vector<float>, std::vector<float>, int, bool skip = false);
	PacketUpdateInfo cpr_path_update(SliceType, std::vector<float>, std::vector<float>, float pathLength, bool skip = false);

	// functions for VR
	PacketUpdateInfo dvr_zoom(int by_ratio, float dx, bool skip = false);
	PacketUpdateInfo dvr_pan(float dx, float dy, bool skip = false);
	PacketUpdateInfo dvr_rotate(float x, float y, int width, int height, bool is_first=false, bool skip = false);
	PacketUpdateInfo dvr_reset();
	PacketUpdateInfo dvr_sculpt(std::vector<int>, std::vector<int>);
	PacketUpdateInfo dvr_voi(std::vector<int>, std::vector<int>);
	PacketUpdateInfo dvr_reset_sculpt();
	PacketUpdateInfo dvr_orientation(int type=0);
	PacketUpdateInfo dvr_projection(int type, std::vector<std::vector<std::vector<int>>>, bool skip=false);
	PacketUpdateInfo dvr_tf(std::map<int, std::vector<int>>, std::vector<std::vector<std::vector<int>>>, bool skip = false);
	PacketUpdateInfo dvr_color(std::map<int, std::vector<int>>, bool skip = false);
	PacketUpdateInfo dvr_shader_switch(int, bool skip = false);
	PacketUpdateInfo dvr_set_lighting(float, float, bool skip = false);

	std::string getErrorMessage() { return errorMsg; }

private:
	// Manager objects
	VolumeMgr* m_volume_mgr;
	MaskMgr* m_mask_mgr;
	GLMgr* m_gl_mgr;
	MPRRectMgr* m_mpr_mgr;
	DVRRectMgr* m_dvr_mgr;
	//MPRMgr* m_mpr_mgr;
	//DVRMgr* m_dvr_mgr;
	CPRMgr* m_cpr_mgr;
	cgip::CgipImageIO* m_io;

	PacketUpdateInfo m_packet_update_info;

	// Rendering Mode
	bool is_mpr_inverse=false;
	bool render_from_server = true;
	SliceType m_cpr_slice_type;

	// IO
	std::string m_save_dir = "E:/temp/";
	std::string m_img_ext = "jpeg";
	std::string errorMsg = " ";
	int m_mpr_sampling_cnt_width, m_mpr_sampling_cnt_height;
	int m_vr_sampling_cnt;
	QDir m_dir;
	int file_num = 0;
	json path_json;

	/* ========================================================================== */
	/* 							   Private Functions							  */
	/* ========================================================================== */	
	std::string _getPathfromSliceType(SliceType);
	SliceType _getSliceTypefromParam(std::string);
};