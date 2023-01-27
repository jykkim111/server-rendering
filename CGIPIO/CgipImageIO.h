#pragma once

#include "cgip_io.h"
#include "CgipFileIO.h"
#include "bitmap_image.hpp"
#include "../CgipCommon/CgipVolume.h"
#include "../CgipCommon/CgipPlane.h"

namespace cgip {
	const int PNG = 1;
	const int JPEG = 2;
	const int RAW = 2;

	class CGIPIO_DLL CgipImageIO : public CgipFileIO {
	public:
		CgipImageIO(std::string ext);

		void readVolume(std::vector<std::string> files) override;
		void readPlane(std::string file) override;
		void saveVolume(CgipVolume& vol) override;
		void savePlane(std::string, CgipPlane&);
		CgipPlane readPlane(std::string, CgipPlane*);
		CgipPlane readMask(std::string, CgipPlane*, std::string);
		CgipMask readMask(std::string, CgipMask*, std::vector<std::string>) override;
		CgipMask readMask(std::vector<std::string>, CgipMask*, std::vector<std::string>) override;

		virtual void saveVolume(CgipMask& mask, std::vector<std::string> file_list) override;

		inline void setImageType(int type) { m_image_type = type; }

	private:
		int m_image_type=0;

		void _savePNGVolume(CgipShort** data, int depth, int height, int width, std::vector<std::string> file_list);
		CgipMask _readPNGMask(std::vector<std::string> files, CgipMask* old_mask);
		void _saveJPEGPlane(CgipPlane&, std::string);
		CgipPlane _readPNG(std::string files, CgipPlane* old_plane);
		CgipPlane _readJPEG(std::string files, CgipPlane* old_plane);
	};
}