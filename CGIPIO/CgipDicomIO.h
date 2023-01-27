#pragma once

#include "cgip_io.h"
#include "CgipFileIO.h"
#include "../CgipCommon/CgipVolume.h"
#include <png.hpp>
#include <fstream>
#include <istream>
#include <streambuf>
#include <numeric>
#include <algorithm>

namespace cgip {

	class CGIPIO_DLL CgipDicomIO : public CgipFileIO{
	public:
		CgipDicomIO();

		void readVolume(std::vector<std::string> files) override;
		void readPlane(std::string file) override;
		void saveVolume(CgipVolume& vol) override;
		void saveVolume(CgipMask& mask, std::vector<std::string> file_list) override;

		CgipMask readMask(std::string file, CgipMask* old_mask, std::vector<std::string> vol_uid_list) override;
		CgipMask readMask(std::vector<std::string>, CgipMask*, std::vector<std::string>) override;


		void readDicom(std::vector<std::string> files);
		inline std::vector<std::string> getUidList() { return m_uid_list; }
		inline int getMatchedSliceCnt() { return m_matched_slice_cnt; }

		/**
		 *	Name:		getImageNumber
		 *	Params:
		 *		- file:	std::string
		 *			path of a dicom file
		 *	Desc:		return image number tag(0020, 0013) value
		 */
		static int getImageNumber(std::string file);
		static bool validateVolume(std::vector<std::string> files, std::string &errorMsg);
		static bool checkInverseOrder(const std::string& file1, const std::string& file2);
		static int checkForIrregular(std::vector<std::string> file);

	private:
		void _readDicom(std::vector<std::string> files);
		//void _readUltraSound(std::vector<std::string> files);

		/**
		 *	Name:		_setPhotometry
		 *	Params:
		 *		- ts_uid:				std::string
		 *			transfer systax uid(Tag: 0002, 0010)
		 *		- samples_per_pixel		int
		 *			the number of data to represent pixel value
		 *	Desc: set photometry interpretation depending on transfer
		 *		systax uid and samples per pixel	
		 */
		//void _setPhotometry(std::string ts_uid, int samples_per_pixel);
		void readDicomHeader(const char* filename);
		void computeSpacingZ(const char* filename1, const char* filename2);

		struct membuf : std::streambuf
		{
			membuf(char* begin, char* end) {
				this->setg(begin, begin, end);
			}
		};

		std::vector<std::string> m_uid_list;
		int m_matched_slice_cnt;
	};

	/**
	 *	Name:	getTagNotFoundMsg
	 *	Params:
	 *		- tag_str:	std::string
	 *			tag index of dicom header
	 *	Desc:	return the error message for failed case to find tag.
	 */
	std::string getTagNotFoundMsg(std::string tag_str);
}
