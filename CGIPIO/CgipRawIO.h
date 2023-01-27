#pragma once

#include "cgip_io.h"
#include "CgipFileIO.h"
#include "../CgipCommon/CgipVolume.h"
#include "../CgipCommon/CgipPlane.h"
#include "../CgipCommon/SegMask.h"

#include <json.hpp>
using json = nlohmann::json;
#include <stdexcept>

namespace cgip {
	class CGIPIO_DLL CgipRawIO : public CgipFileIO{
	public:
		CgipRawIO();

		void readVolume(std::vector<std::string> files) override;
		void readPlane(std::string file) override;
		void saveVolume(CgipVolume& vol) override;
		CgipMask readMask(std::string, CgipMask*, std::vector<std::string>) override;
		CgipMask readMask(std::vector<std::string>, CgipMask*, std::vector<std::string>) override;

		void readVolume(std::string, std::string);
		void readMask(std::string dir_path, CgipMask*);
		SegMask* readMask(std::string dir_path);
		bool saveMask(std::string dir_path, SegMask);

		virtual void saveVolume(CgipMask& mask, std::vector<std::string> file_list) override;

	private:
		void _readRawVolume(std::string file_path);
		void _saveRawVolume(CgipShort** data, int depth, int height, int width, std::vector<std::string> file_list);
	};
}