#include "RenderingFunctionMgr.h"

RenderingFunctionMgr::RenderingFunctionMgr() {
	m_volume_mgr = new VolumeMgr;
	m_mask_mgr = new MaskMgr;
	m_mpr_mgr = new MPRRectMgr;
	m_dvr_mgr = new DVRRectMgr;
	m_gl_mgr = new GLMgr;
	m_cpr_mgr = new CPRMgr;
}

RenderingFunctionMgr::~RenderingFunctionMgr() {
	SAFE_DELETE_OBJECT(m_gl_mgr);
	SAFE_DELETE_OBJECT(m_mpr_mgr);
	SAFE_DELETE_OBJECT(m_dvr_mgr);
	SAFE_DELETE_OBJECT(m_volume_mgr);
	SAFE_DELETE_OBJECT(m_mask_mgr);
	SAFE_DELETE_OBJECT(m_cpr_mgr);
}

bool to_bool(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	std::istringstream is(str);
	bool b;
	is >> std::boolalpha >> b;
	return b;

}

void RenderingFunctionMgr::action_manager(RenderingType rendering_type, ActionType action, json param, bool skip) {
	PacketUpdateInfo info;
	bool is_first;
	if (rendering_type == MPR) {
		switch (action) {
		case LOAD_DATA:
			info = loadVolume(param["value1"], !param["value2"].dump().compare("true"), std::stoi(param["value3"].dump()), std::stoi(param["value4"].dump()));
			break;
		case DATA:
			info.axial_image = true;
			info.sagittal_image = true;
			info.coronal_image = true;
			info.axial_json = true;
			info.sagittal_json = true;
			info.coronal_json = true;
			break;
		case PAN:
			info = mpr_pan(_getSliceTypefromParam(param["value1"]),
				std::stof(param["value2"].dump()),
				std::stof(param["value3"].dump()),
				skip);

			break;
		case ZOOM:
			info = mpr_zoom(_getSliceTypefromParam(param["value1"]),
				std::stoi(param["value2"].dump()),
				std::stof(param["value3"].dump()),
				skip);
			break;
		case WINDOW:
			info = mpr_window(_getSliceTypefromParam(param["value1"]),
				std::stof(param["value2"].dump()),
				std::stof(param["value3"].dump()),
				skip);
			break;
		case RESET:
			info = mpr_reset(_getSliceTypefromParam(param["value1"]));
			break;
		case INVERSE:
			info = mpr_Inverse(_getSliceTypefromParam(param["value1"]), !param["value2"].dump().compare("true"));
			break;
		case MOVE_SLICE:
			info = mpr_move_slice(_getSliceTypefromParam(param["value1"]),
				std::stof(param["value2"].dump()),
				skip);
			break;
		case GUIDELINE_MOVED:
			info = mpr_move_slice(_getSliceTypefromParam(param["value1"]),
				std::stof(param["value2"].dump()),
				std::stof(param["value3"].dump()),
				!param["value4"].dump().compare("true"),
				!param["value5"].dump().compare("true"),
				skip);
			break;
		case OBLIQUE:
			info = mpr_oblique(_getSliceTypefromParam(param["value1"]),
				std::stof(param["value2"].dump()),
				skip);
			break;
		case THICKNESS:
			info = mpr_thickness(_getSliceTypefromParam(param["value1"]),
				std::stoi(param["value2"].dump()),
				skip);
			break;
		case THICKNESS_MODE:
			info = mpr_thickness_mode(std::stoi(param["value1"].dump()), skip);
			break;
		case LOAD_MASK:
			info = loadMask(param["value1"]);
			break;
		case SAVE_MASK:
			info = saveMask(param["value1"]);
			break;
		case MASK_VISIBLE:
			info = mask_visible(std::stoi(param["value1"].dump()),
				!param["value2"].dump().compare("true"));
			break;
		case MASK_OPACITY:
			info = mask_opacity(std::stoi(param["value1"].dump()));
			break;
		case MASK_COLOR:
			info = mask_color(std::stoi(param["value1"].dump()),
				param["value2"]);
			break;
		case RAWP:
			info = mpr_raw(_getSliceTypefromParam(param["value1"]));
			break;
		}
	}
	else if (rendering_type == VR) {
		switch (action) {
		case DATA:
			info.dvr = true;
			break;
		case ZOOM:
			info = dvr_zoom(std::stoi(param["value1"].dump()), std::stof(param["value2"].dump()), skip);
			break;
		case PAN:
			info = dvr_pan(std::stof(param["value1"].dump()), std::stof(param["value2"].dump()), skip);
			break;
		case ROTATE:
			is_first = to_bool(param["value5"].dump());
			info = dvr_rotate(std::stof(param["value1"].dump()), std::stof(param["value2"].dump()), std::stoi(param["value3"].dump()), std::stoi(param["value4"].dump()), is_first, skip);
			break;
		case RESET:
			info = dvr_reset();
			break;
		case SCULPT:
			info = dvr_sculpt(param["value1"], param["value2"]);
			break;
		case VOI:
			info = dvr_voi(param["value1"], param["value2"]);
			break;
		case RESET_SCULPT:
			info = dvr_reset_sculpt();
			break;
		case ORIENTATION:
			info = dvr_orientation(param["value1"]);
			break;
		case PROJECTION:
			if (std::stoi(param["value2"].dump()) == 0) {
				info = dvr_projection(param["value1"], param["value3"], skip);
			}
			else if (std::stoi(param["value2"].dump()) == 2) {
				info = dvr_tf(param["value3"], param["value4"]);
			}
			else {
				info = dvr_color(param["value3"], skip);
			}
			break;
		case MASK_VISIBLE:
			break;
		case MASK_OPACITY:
			break;
		case SETTINGS:
			switch (std::stoi(param["value1"].dump())) {
			case 0:
				info = dvr_shader_switch(std::stoi(param["value2"].dump()), skip);
				break;
			case 1:
				info = dvr_set_lighting(std::stof(param["value2"].dump()), std::stof(param["value3"].dump()), skip);
				break;
			}
			break;
		}
	}
	else {		//rendering_type == CPR
		switch (action) {
		case INIT:
			cpr_init(_getSliceTypefromParam(param["value1"]));
			info.cpr_image = true;
			break;
		case CURVE:
			//info = cpr_curve_update(_getSliceTypefromParam(param["value1"]), param["value2"], param["value3"], skip);
			info = cpr_curve_update(_getSliceTypefromParam(param["value1"]), param["value2"], param["value3"], std::stoi(param["value4"].dump()), skip);
			//info.cpr_curve_json = true;
			break;
		case PATH:
			//info = cpr_path_update(_getSliceTypefromParam(param["value1"]), std::stof(param["value2"].dump()), std::stof(param["value3"].dump()), std::stof(param["value4"].dump()), std::stof(param["value5"].dump()), skip);
			info = cpr_path_update(_getSliceTypefromParam(param["value1"]), param["value2"], param["value3"], std::stof(param["value4"].dump()), skip);
			//info.cpr_path_image = true;
			break;
		}
	}

	m_packet_update_info = info;
}

void RenderingFunctionMgr::cpr_init(SliceType slice_type) {

	m_cpr_mgr->init_CPR(m_gl_mgr, m_volume_mgr, slice_type);
	setCurrentCPRMode(slice_type);

}

PacketUpdateInfo RenderingFunctionMgr::cpr_curve_update(SliceType slice_type, std::vector<float> p_x, std::vector<float> p_y, int numOfPoints, bool skip) {
	PacketUpdateInfo info;

	// set CPR
	if (!skip) {
		m_cpr_mgr->cpr_curve(slice_type, p_x, p_y, numOfPoints);
		m_cpr_mgr->setCPR(slice_type);
	}

	info.cpr_curve_image = true;

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::cpr_path_update(SliceType slice_type, std::vector<float> p_x, std::vector<float> p_y, float pathLength, bool skip) {
	PacketUpdateInfo info;


	if (!skip) {
		// set CPR
		m_cpr_mgr->cpr_path(slice_type, p_x, p_y, pathLength);
		m_cpr_mgr->setCPRPath(slice_type);
	}

	info.cpr_path_image = true;

	return info;
}





PacketUpdateInfo RenderingFunctionMgr::loadVolume(std::string dir, bool is_leaf, int panel_width, int panel_height) {
	PacketUpdateInfo info;

	bool original_mode = true;
	//printf("panel_width: %d\n", panel_width);
	std::string render_error;

	if (!is_leaf) {
		//m_logger_dev->debug("Requested to open volume. {}", dir);
		std::cout << "load volume..: " << dir << std::endl;

		std::vector<std::string> files = cgip::getFileList(dir);

		m_mask_mgr->resetMask();

		bool is_sorted = false;

		printf("file size: %d\n", files.size());

		std::string ext = cgip::checkFilesUnitary(files);
		if (ext.empty()) {
			//m_logger_dev->debug("Mixed directory, only load dicom images.");

			// In case includuiing multiple extension files
			// Only use dicom files
			std::vector<std::string>::iterator it;
			for (it = files.begin(); it != files.end(); ) {
				auto file = *it;
				size_t idx_ext = file.find_last_of('.');
				size_t len_ext = file.length();
				auto ext_each_file = file.substr(idx_ext + 1, len_ext - idx_ext);

				if (!(ext_each_file == "dcm")) { // Not dicom files
					it = files.erase(it);
				}
				else {
					it++;
				}
			}

			// check the extension again
			ext = cgip::checkFilesUnitary(files);
		}

		// Auxilary process for each file format.
		if (ext == "dcm") {
			std::cout << "files are dicom" << std::endl;
			auto cmp_dcm_img_num = [](std::string first, std::string second) -> bool {
				return cgip::CgipDicomIO::getImageNumber(first) < cgip::CgipDicomIO::getImageNumber(second);
			};

			try {
				std::sort(files.begin(), files.end(), cmp_dcm_img_num);
				is_sorted = true;
				printf("sorted by image number\n");
				bool bInverse = false;
				std::vector<bool> vecInverses;

				if (!cgip::CgipDicomIO::validateVolume(files, render_error)) {
					errorMsg = render_error;
					std::cout << errorMsg << std::endl;
					info.error = true;
					return info;
				}


				if (cgip::CgipDicomIO::checkForIrregular(files) == 0) {
					files.erase(files.begin());
				}
				else if (cgip::CgipDicomIO::checkForIrregular(files) == 1) {
					files.pop_back();
				}

				bInverse = cgip::CgipDicomIO::checkInverseOrder(files.at(0), files.at(1));

				if (bInverse) {
					std::reverse(files.begin(), files.end());
				}
				is_sorted = true;
			}
			catch (cgip::CgipUnexistException e) {
				is_sorted = false;
			}
			catch (cgip::CgipUnknownException e) {
				is_sorted = false;
			}
		}
		else {
			//Critical("Not unified extension in files.");
			return info;
		}

		if (!is_sorted) {
			//m_logger_dev->debug("Sort by filename.");

			// Sort by filename
			auto cmp_filename = [](std::string first, std::string second) -> bool {
				return first < second;
			};
			std::sort(files.begin(), files.end(), cmp_filename);
			printf("sorted by fileName\n");
		}

		// load volume

		m_volume_mgr->readVolume(files);

	}
	else {
		size_t idx_ext = dir.find_last_of('.');
		size_t len_ext = dir.length();
		auto before_ext_file = dir.substr(0, idx_ext + 1);
		auto ext_file = dir.substr(idx_ext + 1, len_ext - idx_ext);

		if (ext_file == "vol") {
			m_volume_mgr->readVolume(dir, before_ext_file + "json");
		}

		//m_volume_mgr->readVolume("E:/temp/Volset.vol", "E:/temp/Volmeta.json");
	}

	m_mask_mgr->generateMask(
		m_volume_mgr->getWidth(),
		m_volume_mgr->getHeight(),
		m_volume_mgr->getDepth());

	m_gl_mgr->init_GL(m_volume_mgr, m_mask_mgr);
	//m_gl_mgr->setImageSize(panel_width, panel_height);

	// initialize mpr planes
	int num_channel = m_volume_mgr->getChannel();
	m_mpr_mgr->initSamplingCount(m_mpr_sampling_cnt_width, m_mpr_sampling_cnt_height, num_channel);
	m_mpr_mgr->setOutputImageSize(panel_width, panel_height, num_channel);
	m_mpr_mgr->init_MPR(m_gl_mgr, m_volume_mgr);

	// set plane
	m_mpr_mgr->setAxial();
	m_mpr_mgr->setCoronal();
	m_mpr_mgr->setSagittal();

	// initialize dvr
	//m_dvr_mgr->changeProjection(0, {138, 0, 245, 1596, 255, 1596, 0});
	m_dvr_mgr->initSamplingCount(m_vr_sampling_cnt);
	m_dvr_mgr->setPanelSize(panel_width, panel_height);
	//m_dvr_mgr->sampleByPanel(panel_width, panel_height);
	m_dvr_mgr->init_DVR(m_gl_mgr, m_volume_mgr);

	//server
//#ifdef REMOTE_VIEW
//	m_dvr_mgr->changeProjection(0, { {{130 - m_dvr_mgr->getMinIntensity() , 158}, {242 - m_dvr_mgr->getMinIntensity(), 22}, {1596 - m_dvr_mgr->getMinIntensity(), 22}, {1596 - m_dvr_mgr->getMinIntensity(), 158}} });
//#endif
//	//local
//#ifdef LOCAL_VIEW
//	m_dvr_mgr->changeProjection(0, { {{130, 158}, {242, 22}, {1596, 22}, {1596, 158}} });
//#endif
//	m_dvr_mgr->changeColor({ {286, {255, 255, 255}} });
	m_dvr_mgr->setLighting(0.7, 0.3);
	m_dvr_mgr->init_position();
	m_dvr_mgr->setDVR();

	m_cpr_mgr->initSamplingCount(m_mpr_sampling_cnt_width, m_mpr_sampling_cnt_height, num_channel);


	info.axial_image = true;
	info.coronal_image = true;
	info.sagittal_image = true;
	info.axial_json = true;
	info.coronal_json = true;
	info.sagittal_json = true;
	info.dvr = true;
	info.cpr_image = false;
	info.cpr_curve_image = false;
	info.cpr_path_image = false;
	info.volume_json = true;

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::loadMask(std::vector<std::string> dir) {
	PacketUpdateInfo info;
	m_mask_mgr->loadMaskData(dir);
	m_gl_mgr->setMaskTexture();

	// set MPR
	//m_mpr_mgr->updateMask();
	m_mpr_mgr->setAxial();
	m_mpr_mgr->setCoronal();
	m_mpr_mgr->setSagittal();

	// set DVR
	//m_dvr_mgr->updateMask();
	m_dvr_mgr->setDVR();

	// update
	info.axial_image = true;
	info.coronal_image = true;
	info.sagittal_image = true;
	info.dvr = true;
	info.mask_json = true;

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::saveMask(std::string dir) {
	PacketUpdateInfo info;
	m_mask_mgr->saveMaskData(dir);

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_Inverse(SliceType slice_type, bool v) {
	PacketUpdateInfo info;

	m_mpr_mgr->inverse(slice_type, v);

	// set MPR
	switch (slice_type) {
	case AXIAL_SLICE:
		m_mpr_mgr->setAxial();
		info.axial_image = true;
		break;
	case CORONAL_SLICE:
		m_mpr_mgr->setCoronal();
		info.coronal_image = true;
		break;
	case SAGITTAL_SLICE:
		m_mpr_mgr->setSagittal();
		info.sagittal_image = true;
		break;
	}

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::mpr_zoom(SliceType slice_type, int is_ratio, float d, bool skip) {
	int result = 0;
	if (is_ratio) {
		result = m_mpr_mgr->zoom_slice_ratio(slice_type, m_volume_mgr, d);
	}
	else {
		result = m_mpr_mgr->zoom_slice(slice_type, m_volume_mgr, d);
	}
	PacketUpdateInfo info;
	if (result <= 0) return info;

	// update
	if (!skip) {
		switch (slice_type) {
		case AXIAL_SLICE:
			info.axial_image = true;
			info.axial_json = true;
			m_mpr_mgr->setAxial();
			break;
		case CORONAL_SLICE:
			info.coronal_image = true;
			info.coronal_json = true;
			m_mpr_mgr->setCoronal();
			break;
		case SAGITTAL_SLICE:
			info.sagittal_image = true;
			info.sagittal_json = true;
			m_mpr_mgr->setSagittal();
			break;
		}
	}

	return info;
}


PacketUpdateInfo RenderingFunctionMgr::mpr_pan(SliceType slice_type, float dx, float dy, bool skip) {
	int result = m_mpr_mgr->slice_panning(slice_type, m_volume_mgr, dx, dy);

	PacketUpdateInfo info;
	if (result <= 0) return info;

	// update
	if (!skip) {
		switch (slice_type) {
		case AXIAL_SLICE:
			info.axial_image = true;
			info.axial_json = true;
			m_mpr_mgr->setAxial();
			break;
		case CORONAL_SLICE:
			info.coronal_image = true;
			info.coronal_json = true;
			m_mpr_mgr->setCoronal();
			break;
		case SAGITTAL_SLICE:
			info.sagittal_image = true;
			info.sagittal_json = true;
			m_mpr_mgr->setSagittal();
			break;
		}
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_window(SliceType slice_type, float dx, float dy, bool skip) {
	int result = m_mpr_mgr->windowing(slice_type, dx, dy);

	PacketUpdateInfo info;
	if (result <= 0) return info;

	if (!skip) {
		switch (slice_type) {
		case AXIAL_SLICE:
			m_mpr_mgr->setAxial();
			info.axial_image = true;
			info.axial_json = true;
			break;
		case CORONAL_SLICE:
			m_mpr_mgr->setCoronal();
			info.coronal_image = true;
			info.coronal_json = true;
			break;
		case SAGITTAL_SLICE:
			m_mpr_mgr->setSagittal();
			info.sagittal_image = true;
			info.sagittal_json = true;
			break;
		}
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_center(SliceType slice_type, float x, float y) {
	int result = m_mpr_mgr->align_center(slice_type, m_volume_mgr, x, y);

	PacketUpdateInfo info;
	if (result <= 0) return info;

	m_mpr_mgr->setAxial();
	m_mpr_mgr->setCoronal();
	m_mpr_mgr->setSagittal();

	// update
	info.axial_image = true;
	info.coronal_image = true;
	info.sagittal_image = true;
	info.axial_json = true;
	info.coronal_json = true;
	info.sagittal_json = true;

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_reset(SliceType slice_type) {
	m_mpr_mgr->init_position(slice_type);

	// update
	PacketUpdateInfo info;
	switch (slice_type) {
	case AXIAL_SLICE:
		info.axial_image = true;
		info.axial_json = true;
		info.coronal_json = true;
		info.sagittal_json = true;
		m_mpr_mgr->setAxial();
		break;
	case CORONAL_SLICE:
		info.coronal_image = true;
		info.axial_json = true;
		info.coronal_json = true;
		info.sagittal_json = true;
		m_mpr_mgr->setCoronal();
		break;
	case SAGITTAL_SLICE:
		info.sagittal_image = true;
		info.axial_json = true;
		info.coronal_json = true;
		info.sagittal_json = true;
		m_mpr_mgr->setSagittal();
		break;
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_move_slice(SliceType slice_type, float x, float y, bool h, bool v, bool skip) {
	int result = m_mpr_mgr->move_slice(slice_type, m_volume_mgr, x, y, h, v);

	// update
	PacketUpdateInfo info;
	if (result <= 0) return info;

	if (!skip) {
		switch (slice_type) {
		case AXIAL_SLICE:
			info.coronal_image = true;
			info.sagittal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setCoronal();
			m_mpr_mgr->setSagittal();
			break;
		case CORONAL_SLICE:
			info.axial_image = true;
			info.sagittal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setAxial();
			m_mpr_mgr->setSagittal();
			break;
		case SAGITTAL_SLICE:
			info.axial_image = true;
			info.coronal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setAxial();
			m_mpr_mgr->setCoronal();
			break;
		}
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_move_slice(SliceType slice_type, float d, bool skip) {
	int result = m_mpr_mgr->scroll_slice(slice_type, m_volume_mgr, d);

	// update
	PacketUpdateInfo info;
	if (result <= 0) return info;

	if (!skip) {
		switch (slice_type) {
		case AXIAL_SLICE:
			info.axial_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setAxial();
			break;
		case CORONAL_SLICE:
			info.coronal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setCoronal();
			break;
		case SAGITTAL_SLICE:
			info.sagittal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setSagittal();
			break;
		}
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_oblique(SliceType slice_type, float angle, bool skip) {
	int result = m_mpr_mgr->rotate_slice(slice_type, angle);

	// update
	PacketUpdateInfo info;
	if (result <= 0) return info;

	if (!skip) {
		switch (slice_type) {
		case AXIAL_SLICE:
			info.coronal_image = true;
			info.sagittal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setCoronal();
			m_mpr_mgr->setSagittal();
			break;
		case CORONAL_SLICE:
			info.axial_image = true;
			info.sagittal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setAxial();
			m_mpr_mgr->setSagittal();
			break;
		case SAGITTAL_SLICE:
			info.axial_image = true;
			info.coronal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setAxial();
			m_mpr_mgr->setCoronal();
			break;
		}
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_thickness(SliceType slice_type, int v, bool skip) {
	m_mpr_mgr->thickness(slice_type, v);

	// update
	PacketUpdateInfo info;

	if (!skip) {
		switch (slice_type) {
		case AXIAL_SLICE:
			info.axial_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setAxial();
			break;
		case CORONAL_SLICE:
			info.coronal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setCoronal();
			break;
		case SAGITTAL_SLICE:
			info.sagittal_image = true;
			info.axial_json = true;
			info.coronal_json = true;
			info.sagittal_json = true;
			m_mpr_mgr->setSagittal();
			break;
		}
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_thickness_mode(int v, bool skip) {
	m_mpr_mgr->thickness_mode(v);

	// update
	PacketUpdateInfo info;

	if (!skip) {
		info.axial_image = true;
		info.coronal_image = true;
		info.sagittal_image = true;
		m_mpr_mgr->setAxial();
		m_mpr_mgr->setSagittal();
		m_mpr_mgr->setCoronal();
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mask_visible(int idx, bool v) {
	PacketUpdateInfo info;

	m_mask_mgr->setVisible(idx, v);
	//m_gl_mgr->setMaskTexture();

	//m_mpr_mgr->updateMask();
	m_mpr_mgr->setAxial();
	m_mpr_mgr->setSagittal();
	m_mpr_mgr->setCoronal();

	//m_dvr_mgr->updateMask();
	m_dvr_mgr->setDVR();

	info.axial_image = true;
	info.coronal_image = true;
	info.sagittal_image = true;
	info.dvr = true;

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mask_opacity(int v) {
	PacketUpdateInfo info;

	m_mask_mgr->setOpacity(v);

	m_mpr_mgr->setAxial();
	m_mpr_mgr->setSagittal();
	m_mpr_mgr->setCoronal();

	m_dvr_mgr->setDVR();

	info.axial_image = true;
	info.coronal_image = true;
	info.sagittal_image = true;
	info.dvr = true;

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mask_color(int idx, std::string v) {
	PacketUpdateInfo info;

	m_mask_mgr->setColor(idx, v);

	m_mpr_mgr->setAxial();
	m_mpr_mgr->setSagittal();
	m_mpr_mgr->setCoronal();

	m_dvr_mgr->setDVR();

	info.axial_image = true;
	info.coronal_image = true;
	info.sagittal_image = true;
	info.dvr = true;

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::mpr_raw(SliceType slice_type) {
	m_mpr_mgr->setRawMPR(slice_type);

	PacketUpdateInfo info;
	// for test
	/*
	cgip::CgipPlane temp = m_mpr_mgr->getRaw();
	m_io = new cgip::CgipImageIO("jpeg");
	m_io->setImageType(cgip::JPEG);
	m_io->savePlane("./temp.jpeg", temp);
	*/
	return info;
}


PacketUpdateInfo RenderingFunctionMgr::dvr_zoom(int by_ratio, float dx, bool skip) {

	if (by_ratio == 1) {
		m_dvr_mgr->zoom_by_ratio(dx);
	}
	else {
		m_dvr_mgr->zooming(dx);
	}


	PacketUpdateInfo info;

	if (!skip) {
		m_dvr_mgr->setDVR();
		info.dvr = true;
	}

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::dvr_pan(float dx, float dy, bool skip) {
	m_dvr_mgr->panning(dx, -dy);

	PacketUpdateInfo info;
	if (!skip) {
		m_dvr_mgr->setDVR();
		info.dvr = true;
	}

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::dvr_rotate(float x, float y, int width, int height, bool is_first, bool skip) {
	PacketUpdateInfo info;
	if (is_first) {
		m_dvr_mgr->setStartPosition(x, y);
		return info;
	}

	m_dvr_mgr->rotation(x, y, width, height);

	if (!skip) {
		m_dvr_mgr->setDVR();
		info.dvr = true;
	}

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::dvr_reset() {
	m_dvr_mgr->init_position();
	m_dvr_mgr->setDVR();

	PacketUpdateInfo info;
	info.dvr = true;

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::dvr_sculpt(std::vector<int> points_x, std::vector<int> points_y) {
	m_dvr_mgr->doSculpt(points_x, points_y);
	m_dvr_mgr->setDVR();

	PacketUpdateInfo info;
	info.dvr = true;

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::dvr_voi(std::vector<int> points_x, std::vector<int> points_y) {
	m_dvr_mgr->doVOI(points_x, points_y);
	m_dvr_mgr->setDVR();

	PacketUpdateInfo info;
	info.dvr = true;

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::dvr_reset_sculpt() {
	m_dvr_mgr->resetSculpt();
	m_dvr_mgr->setDVR();

	PacketUpdateInfo info;
	info.dvr = true;

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::dvr_orientation(int type) {
	m_dvr_mgr->init_geometry((VROrientationType)type);
	m_dvr_mgr->setDVR();

	PacketUpdateInfo info;
	info.dvr = true;

	return info;
};

PacketUpdateInfo RenderingFunctionMgr::dvr_projection(int type, std::vector<std::vector<std::vector<int>>> coords, bool skip) {
	PacketUpdateInfo info;

	if (!skip) {
		m_dvr_mgr->changeProjection(type, coords);
		m_dvr_mgr->setDVR();

		info.dvr = true;
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::dvr_color(std::map<int, std::vector<int>> colorMap, bool skip) {
	PacketUpdateInfo info;
	m_dvr_mgr->changeColor(colorMap);

	if (!skip) {
		m_dvr_mgr->setDVR();

		info.dvr = true;
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::dvr_tf(std::map<int, std::vector<int>> colorMap, std::vector<std::vector<std::vector<int>>> coords, bool skip) {
	PacketUpdateInfo info;

	m_dvr_mgr->changeColor(colorMap);
	m_dvr_mgr->changeProjection(0, coords);

	if (!skip) {
		m_dvr_mgr->setDVR();

		info.dvr = true;
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::dvr_shader_switch(int status, bool skip) {
	PacketUpdateInfo info;
	m_dvr_mgr->setShaderStatus(status);

	if (!skip) {
		m_dvr_mgr->setDVR();

		info.dvr = true;
	}

	return info;
}

PacketUpdateInfo RenderingFunctionMgr::dvr_set_lighting(float specular_val, float brightness_val, bool skip) {
	PacketUpdateInfo info;
	m_dvr_mgr->setLighting(specular_val, brightness_val);

	if (!skip) {
		m_dvr_mgr->setDVR();

		info.dvr = true;
	}

	return info;
}

void RenderingFunctionMgr::initSamplingCnt(int mpr_w, int mpr_h, int vr) {
	m_mpr_sampling_cnt_height = mpr_h;
	m_mpr_sampling_cnt_width = mpr_w;
	m_vr_sampling_cnt = vr;
}

void RenderingFunctionMgr::setMPRSamplingCnt(int w, int h, bool is_downsampling, bool skip) {
	m_mpr_mgr->setSamplingCount(w, h, m_volume_mgr->getChannel(), is_downsampling);
	//m_mpr_mgr->init_MPR(m_volume_mgr, m_mask_mgr);

	if (!skip) {
		m_mpr_mgr->setAxial();
		m_mpr_mgr->setCoronal();
		m_mpr_mgr->setSagittal();
	}

	m_mpr_sampling_cnt_height = h;
	m_mpr_sampling_cnt_width = w;

	if (!render_from_server) {
		PacketUpdateInfo info;
		info.axial_image = true;
		info.coronal_image = true;
		info.sagittal_image = true;

		m_packet_update_info = info;
	}


#ifdef LOCAL_VIEW
	PacketUpdateInfo info;
	info.axial_image = true;
	info.coronal_image = true;
	info.sagittal_image = true;

	m_packet_update_info = info;
#endif
}

void RenderingFunctionMgr::setVRSamplingCnt(int c, bool is_downsampling, bool skip) {
	m_dvr_mgr->setSamplingCount(c, is_downsampling);


	if (!skip) {
		m_dvr_mgr->setDVR();
	}
	m_vr_sampling_cnt = c;

#ifdef LOCAL_VIEW
	PacketUpdateInfo info;
	info.dvr = true;

	m_packet_update_info = info;
#endif
}

void RenderingFunctionMgr::setCPRSamplingCnt(int w, int h, int mode, SliceType slice_type, bool skip) {
	m_cpr_mgr->setSamplingCount(w, h, m_volume_mgr->getChannel());

}



// Get Json Format Info
json RenderingFunctionMgr::getPacketUpdateInfobyJson() {
	return { {"axial_image", m_packet_update_info.axial_image},
			  {"coronal_image", m_packet_update_info.coronal_image},
			  {"sagittal_image", m_packet_update_info.sagittal_image},
			  {"cpr_image", m_packet_update_info.cpr_image},
			  {"cpr_curve_image", m_packet_update_info.cpr_curve_image},
			  {"cpr_path_image", m_packet_update_info.cpr_path_image},
			  {"axial_json", m_packet_update_info.axial_json},
			  {"coronal_json", m_packet_update_info.coronal_json},
			  {"sagittal_json", m_packet_update_info.sagittal_json},
			  {"dvr", m_packet_update_info.dvr},
			  {"mask_json", m_packet_update_info.mask_json},
			  {"volume_json", m_packet_update_info.volume_json},
			  {"error", m_packet_update_info.error}
	};
}
json RenderingFunctionMgr::getVolumeInfobyJson() {
	VolumePacket p = m_volume_mgr->getVolumePacket();
	return {
		{"volume", {
			{"ImageRescaleIntercept", p.ImageRescaleIntercept},
			{"ImageRescaleSlope", p.ImageRescaleSlope},
			{"PixelRepresentation", p.PixelRepresentation},
			{"PhotometricInterpretation", p.PhotometricInterpretation},
			{"ImageWindowWidth", p.ImageWindowWidth},
			{"ImageWindowCenter", p.ImageWindowCenter},
			{"ImageCols", p.ImageCols},
			{"ImageRows", p.ImageRows},
			{"PixelSpacingRow", p.PixelSpacingRow},
			{"PixelSpacingCol", p.PixelSpacingCol},
			{"BitsStored", p.BitsStored},
			{"BitsAllocated", p.BitsAllocated},
			{"SliceThickness", p.SliceThickness},
			{"MaxVal", p.MaxVal},
			{"MinVal", p.MinVal},
			}
		}
	};
}
json RenderingFunctionMgr::getMPRInfobyJson(SliceType s) {
	SlicePacket slice_packet = m_mpr_mgr->getSlicePacket(s);

	return {
		{"mpr", {
			{"slice_line", slice_packet.slice_line},
			{"thickness", slice_packet.thickness},
			{"center_point", slice_packet.center_point},
			{"lefttop_point", slice_packet.lefttop_point},
			{"normal_vector", slice_packet.normal_vector},
			{"width_direction_vetor", slice_packet.width_direction_vetor},
			{"height_direction_vector", slice_packet.height_direction_vector},
			{"pixel_space", slice_packet.pixel_space},
			{"image_num", slice_packet.image_num},
			{"m_axis_size", slice_packet.m_axis_size },
			{"panning", slice_packet.panning},
			{"image_size", slice_packet.image_size}
			}}
	};
}
json RenderingFunctionMgr::getVRInfobyJson() {
	DVRPacket dvr_packet = m_dvr_mgr->getDVRPacket();
	return {
		{"dvr", {
			{"lefttop_point", dvr_packet.lefttop_point},
			{"normal_vector", dvr_packet.normal_vector},
			{"width_direction_vetor", dvr_packet.width_direction_vetor},
			{"height_direction_vector", dvr_packet.height_direction_vector},
			{"pixel_space", dvr_packet.pixel_space},
			{"m_axis_size", dvr_packet.m_axis_size},
			{"image_size", dvr_packet.image_size}
			}
		},
	};
}

json RenderingFunctionMgr::getCPRCurveInfobyJson(SliceType s) {
	CPRCurvePacket curve_packet = m_cpr_mgr->getCurvePacket(s);
	return {
		{"dvr", {
			{"lefttop_point", curve_packet.lefttop_point},
			{"normal_vector", curve_packet.normal_vector},
			{"width_direction_vetor", curve_packet.width_direction_vetor},
			{"height_direction_vector", curve_packet.height_direction_vector},
			{"pixel_space", curve_packet.pixel_space},
			{"m_axis_size", curve_packet.m_zoom_factor},
			}
		},
	};
}

//json RenderingFunctionMgr::getCPRPathInfobyJson() {
//	DVRPacket dvr_packet = m_cpr_mgr->getPathPacket();
//	return {
//		{"dvr", {
//			{"lefttop_point", dvr_packet.lefttop_point},
//			{"normal_vector", dvr_packet.normal_vector},
//			{"width_direction_vetor", dvr_packet.width_direction_vetor},
//			{"height_direction_vector", dvr_packet.height_direction_vector},
//			{"pixel_space", dvr_packet.pixel_space},
//			{"m_axis_size", dvr_packet.m_axis_size},
//			}
//		},
//	};
//}
//


json RenderingFunctionMgr::getMaskInfobyJson() {
	MaskLabel* mask_label_info = m_mask_mgr->getMaskLabel();
	json mask_json_info;

	for (int i = 1; i <= G_MAX_MASK_NUM; i++) {
		mask_json_info["mask"][std::to_string(i)]["is_empty"] = mask_label_info[i].is_empty;
		mask_json_info["mask"][std::to_string(i)]["is_visible"] = mask_label_info[i].is_visible;
		mask_json_info["mask"][std::to_string(i)]["name"] = mask_label_info[i].name;
		mask_json_info["mask"][std::to_string(i)]["color"] = mask_label_info[i].color;
	}

	return mask_json_info;
}
cgip::CgipPlane RenderingFunctionMgr::getMPRImage(SliceType s) {
	switch (s) {
	case AXIAL_SLICE:
		return m_mpr_mgr->getAxial();
	case CORONAL_SLICE:
		return m_mpr_mgr->getCoronal();
	case SAGITTAL_SLICE:
		return m_mpr_mgr->getSagittal();
	}
}
cgip::CgipPlane RenderingFunctionMgr::getVRImage() {
	return m_dvr_mgr->getDVR();
}

cgip::CgipPlane RenderingFunctionMgr::getCPRImage() {
	return m_cpr_mgr->getCPR();
}

cgip::CgipPlane RenderingFunctionMgr::getCPRPathImage() {
	return m_cpr_mgr->getCPRPath();
}


void RenderingFunctionMgr::write_jpeg_file(cgip::CgipPlane* plane, const char* filename, int quality) {

	int length = plane->getWidth() * plane->getHeight() * plane->getChannel();
	std::vector<CgipShort> data = plane->getPlane();
	unsigned char* image_buffer = (unsigned char*)malloc(length);

	int image_width = plane->getWidth();
	int image_height = plane->getHeight();

#pragma omp parallel for
	for (int i = 0; i < length; i++) {
		image_buffer[i] = data[i];
	}

	unsigned long jSize = 0;
	unsigned char* jBuf = NULL;



	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE* outfile;                /* target file */
	JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
	int row_stride = 0;               /* physical row width in image buffer */

	/* Step 1: allocate and initialize JPEG compression object */


	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */


	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */


	cinfo.image_width = image_width;      /* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;           /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */

	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	 * Here we just illustrate the use of quality (quantization table) scaling:
	 */
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	 * Pass TRUE unless you are very sure of what you're doing.
	 */
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */


	row_stride = image_width * cinfo.input_components; /* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient.
		 */
		row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);
}



/* ========================================================================== */
/* 							   Private Functions							  */
/* ========================================================================== */

SliceType RenderingFunctionMgr::_getSliceTypefromParam(std::string slice_type) {
	if (slice_type == "Axial") {
		return AXIAL_SLICE;
	}
	else if (slice_type == "Coronal") {
		return CORONAL_SLICE;
	}
	else if (slice_type == "Sagittal") {
		return SAGITTAL_SLICE;
	}
}

std::string RenderingFunctionMgr::_getPathfromSliceType(SliceType slice_type) {
	if (slice_type == AXIAL_SLICE) {
		return m_save_dir + "axial";
	}
	else if (slice_type == CORONAL_SLICE) {
		return m_save_dir + "coronal";
	}
	else if (slice_type == SAGITTAL_SLICE) {
		return m_save_dir + "sagittal";
	}
}