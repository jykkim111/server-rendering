#include "MaskMgr.h"

#include <Windows.h>
#include <iostream>

MaskMgr::MaskMgr() {
	m_io = nullptr;
}

MaskMgr::MaskMgr(int width, int height, int depth) {
	generateMask(width, height, depth);
}

MaskMgr::~MaskMgr() {
	SAFE_DELETE_ARRAY(m_labels);
	SAFE_DELETE_OBJECT(m_io);
}

void MaskMgr::loadMaskData(std::vector<std::string> dir_paths) {
	m_io = new cgip::CgipRawIO();

	resetMask();

	int num_dir = dir_paths.size();
	for (int i = 0; i < std::min(8, num_dir); i++) {
		cgip::SegMask* new_mask = ((cgip::CgipRawIO*)m_io)->readMask(dir_paths[i]);
		if (new_mask->getWidth() >= m_mask[i].getWidth() &&
			new_mask->getHeight() >= m_mask[i].getHeight() &&
			new_mask->getDepth() >= m_mask[i].getDepth()) {

			m_mask[i] = *new_mask;
		}
		SAFE_DELETE_OBJECT(new_mask);
		
		m_labels[i+1].is_empty = false;
		int start = dir_paths[i].find_last_of("/");
		std::string filename = dir_paths[i].substr(start + 1);
		int end = filename.find_last_of(".");
		m_labels[i+1].name = filename.substr(0, end);
		m_labels[i+1].color = m_mask_color_table[i+1];
		m_mask_cnt = m_mask_cnt + 1;
	}
}

bool MaskMgr::saveMaskData(std::string dir_path) {
	m_io = new cgip::CgipRawIO();

	for (int i = 0; i < m_mask_cnt;i++) {
		std::string save_path = dir_path + "/" + m_labels[i + 1].name + ".dat";
		((cgip::CgipRawIO*)m_io)->saveMask(save_path, m_mask[i]);
	}

	return true;
}

void MaskMgr::addMaskLabel(const char* name) {
	// Cannot overflow
	if (m_mask_cnt == MAX_MASK_NUM) return;

	// Find empty label
	int idx = 1;
	for (; idx <= MAX_MASK_NUM; idx++) {
		if (m_labels[idx].is_empty) break;
	}

	// Set label info
	m_labels[idx].is_empty = false;
	m_labels[idx].name = name;
	m_labels[idx].color = m_mask_color_table[idx];
	m_labels[idx].is_visible = true;

	m_mask_cnt = m_mask_cnt + 1;
}

cgip::CgipPlane MaskMgr::getViewingPlane(
	cgip::CgipPoint& topleft_p,
	cgip::CgipVector w_dir,
	cgip::CgipVector h_dir,
	int w_cnt,
	int h_cnt,
	float w_len,
	float h_len,
	float sp_x,
	float sp_y,
	float sp_z) {

	cgip::CgipPlane mask(w_cnt, h_cnt);

#pragma omp parallel for
		for (int xy = 0; xy < w_cnt * h_cnt; xy++) {
			int x = xy / w_cnt;
			int y = xy % w_cnt;

			cgip::CgipPoint p_cur = topleft_p + x * w_len * w_dir + y * h_len * h_dir;

			mask.setPixelValue(x, y, BACKGROUND_IDX);

			for (int i = 0; i < m_mask_cnt; i++) {
				//if (m_labels[i+1].is_visible == false) continue;

				cgip::E_MaskValue val = m_mask[i].getVoxelValue(
					p_cur.x() / sp_x,
					p_cur.y() / sp_y,
					p_cur.z() / sp_z);

				// Set background label if the label is not visible
				if (val==cgip::ONE) {
					mask.setPixelValue(x, y, i+1);
					break;
				}
			}
		}

	return mask;
}

cgip::CgipMask MaskMgr::getViewingVolume(int w, int h, int d) {

	cgip::CgipMask mask(w, h, d);

#pragma omp parallel for
	for (int xyz = 0; xyz < w * h * d; xyz++) {
		int z = xyz / (w * h);
		int y = (xyz % (w * h)) / w;
		int x = (xyz % (w * h)) % w;

		mask.setVoxelValue(x, y, z, BACKGROUND_IDX);

		for (int i = 0; i < m_mask_cnt; i++) {
			if (m_labels[i + 1].is_visible == false) continue;

			cgip::E_MaskValue val = m_mask[i].getVoxelValue(x, y, z);

			// Set background label if the label is not visible
			if (val == cgip::ONE) {
				mask.setVoxelValue(x, y, z, i + 1);
				break;
			}
		}
	}

	return mask;
}

void MaskMgr::changeLabelVisible(int idx) {
	m_labels[idx].is_visible = !m_labels[idx].is_visible;
}

void MaskMgr::generateMask(int width, int height, int depth) {
	// Create a mask object

	m_mask.clear();

	for (int i = 0; i < 8; i++) {
		cgip::SegMask mask(width, height, depth);
		m_mask.push_back(mask);
	}

	// Initialize labels
	m_mask_cnt = 0;
	m_cur_mask_idx = BACKGROUND_IDX;

	for (int i = 0; i <= MAX_MASK_NUM; i++) {
		m_labels[i].is_empty = true;
		m_labels[i].is_visible = true;
	}

	// Background as first label
	m_labels[0].is_empty = false;
	m_labels[0].name = "Background";

	// TODO: set mask color table with param
	//emit sigChangeMaskColorTable(m_mask_color_table);
}

void MaskMgr::resetMask() {
	// Initialize labels
	m_mask_cnt = 0;
	m_cur_mask_idx = BACKGROUND_IDX;

	for (int i = 0; i < MAX_MASK_NUM + 1; i++) {
		m_labels[i].is_empty = true;
		m_labels[i].is_visible = true;
	}

	m_labels[0].is_empty = false;
	m_labels[0].name = "Background";
}

int MaskMgr::getLargestMaskLoc() {
	// return location of axial slice where its mask is the largest
	int max_mask_size = 0;
	int max_mask_idx = 0;

	/*
	for (int i = 0; i < m_mask_cnt; i++) {
		for (int z = 0; z < m_mask[i]->getDepth(); z++) {
			int cur_mask_size = 0;
			for (int y = 0; y < m_mask[i]->getHeight(); y++) {
				for (int x = 0; x < m_mask[i]->getWidth(); x++) {
					if (m_mask[i]->getVoxelValue(x, y, z) > 0) {
						cur_mask_size++;
					}
				}
			}

			if (max_mask_size < cur_mask_size) {
				max_mask_size = cur_mask_size;
				max_mask_idx = z;
			}
		}
	}
	if (max_mask_size == 0)
		return -1;
	*/
	return max_mask_idx;
}

/* ========================================================================== */
/*									QT Slots								  */
/* ========================================================================== */
void MaskMgr::slotReqAddMaskLabel(const char* name) {
	addMaskLabel(name);
}

/*
void MaskMgr::slotReqChangeCurMaskLabel(int idx) {
	changeCurLabel(idx);
}
*/

void MaskMgr::slotReqChangeMaskVisible(int idx) {
	changeLabelVisible(idx);
}

/*
void MaskMgr::slotReqEraseMaskLabel(int idx) {
	eraseMaskLabel(idx);
}
*/
