#pragma once


#include <QObject>

#include "../CGIPIO/CgipFileIO.h"
#include "../CGIPIO/CgipDicomIO.h"
#include "../CGIPIO/CgipRawIO.h"
#include "../CGIPIO/CgipImageIO.h"
#include "../CGIPIO/MaskInfo.h"
#include "../CgipCommon/CgipMask.h"
#include "../CgipCommon/SegMask.h"
#include "../CgipCommon/CgipPlane.h"
#include "../CgipException/CgipException.h"

#include "viewermanager.h"
#include "global.h"
#include "Mgr.h"

class VIEWERMANAGER_DLL MaskMgr : public Mgr{
	Q_OBJECT

public slots:
	void slotReqAddMaskLabel(const char* name="noname");
	//void slotReqChangeCurMaskLabel(int idx);
	void slotReqChangeMaskVisible(int idx);
	//void slotReqEraseMaskLabel(int idx);

public:
	MaskMgr();
	MaskMgr(int width, int height, int depth);
	~MaskMgr();

	void loadMaskData(std::vector<std::string> dir_paths);
	bool saveMaskData(std::string dir_path);

	// Access current idx
	inline int getMaskIdx() { return m_cur_mask_idx; }

	inline void setColor(int idx, std::string c) { m_mask_color_table[idx] = c; }
	inline void setVisible(int idx, bool v) { m_labels[idx].is_visible = v; }
	inline void setOpacity(int v) { m_opacity = v; }

	inline int getOpacity() { return m_opacity; }
	inline std::string getColor(int idx) { return m_mask_color_table[idx]; }
	inline bool getVisibility(int label) { return m_labels[label].is_visible; }
	inline MaskLabel* getMaskLabel() { return m_labels; }
	inline int getMatchedSliceCnt() { return m_matched_slice_cnt; }

	// Create empty mask at initial.
	void generateMask(int width, int height, int depth);
	void resetMask();

	void addMaskLabel(const char* name="noname");

	void changeLabelVisible(int idx);

	int getLargestMaskLoc();

	cgip::CgipMask getViewingVolume(int w, int h, int d);

	cgip::CgipPlane getViewingPlane(
		cgip::CgipPoint& topleft_p,
		cgip::CgipVector w_dir,
		cgip::CgipVector h_dir,
		int w_cnt,
		int h_cnt,
		float w_len,
		float h_len,
		float sp_x,
		float sp_y,
		float sp_z);

private:
	cgip::CgipFileIO* m_io;
	std::vector<cgip::SegMask> m_mask;

	const int BACKGROUND_IDX = 0;
	const int MAX_MASK_NUM = G_MAX_MASK_NUM;

	int m_mask_cnt = 0;
	int m_cur_mask_idx = BACKGROUND_IDX;
	int m_matched_slice_cnt = 0;
	int m_opacity = 5;

	std::vector<std::string> m_mask_color_table = default_mask_color_table;

	MaskLabel* m_labels = new MaskLabel[MAX_MASK_NUM + 1];
};