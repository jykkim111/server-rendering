#pragma once

#include <vector>
#include <string>
#include <sys/stat.h>

#include "cgip_io.h"

#include "../CgipCommon/CgipVolume.h"
#include "../CgipCommon/CgipMask.h"
#include "../CgipException/CgipException.h"


namespace cgip {

	const int BASE64_DECODE_TABLE[256] = {
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
			52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
			-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
			15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
			-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
			41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
	};

	typedef struct VolumeInfo {
		// Photometric interpretation
		std::string photometry;

		CgipInt samples_per_pixel = 1; 	// Todo: replaced with

		// Volume dimension(voxel unit) information.
		CgipInt width	= 0;
		CgipInt height	= 0;
		CgipInt depth	= 0;

		CgipFloat rescale_intercept	= 0;
		CgipFloat rescale_slope		= 1;

		// Spacing information(mm unit) between voxels.
		CgipFloat spacing_x = 1;
		CgipFloat spacing_y = 1;
		CgipFloat spacing_z = 1;

		CgipInt default_window_center = 0;
		CgipInt default_window_width = 0;
		std::string modality = "None"; // "CT", "MR", "US", "None" TODO: change to enum
		std::string series_uid = "NULL";
		std::string patient_position = "HFS";

		CgipFloat m_ptHor_x = 0;
		CgipFloat m_ptHor_y = 0;
		CgipFloat m_ptHor_z = 0;
		CgipFloat m_ptVer_x = 0;
		CgipFloat m_ptVer_y = 0;
		CgipFloat m_ptVer_z = 0;

		CgipFloat m_FirstImg_x = 0, m_FirstImg_y = 0, m_FirstImg_z = 0;
		CgipFloat m_SecondImg_x = 0, m_SecondImg_y = 0, m_SecondImg_z = 0;
		CgipFloat m_LastImg_x = 0, m_LastImg_y = 0, m_LastImg_z = 0;
	}VolumeInfo;

	/*
	This is an interface class for FileIO classes
	*/
	class CGIPIO_DLL CgipFileIO {
	public:
		CgipFileIO();

		inline void setExt(std::string ext) { m_ext = ext; }
		inline std::string getExt() { return m_ext; }

		virtual void readVolume(std::vector<std::string> files) = 0;
		virtual void readPlane(std::string file) = 0;
		virtual void saveVolume(CgipVolume& vol) = 0;
		virtual void saveVolume(CgipMask& mask, std::vector<std::string> file_list) = 0;
		virtual CgipMask readMask(std::string, CgipMask*, std::vector<std::string>) = 0;
		virtual CgipMask readMask(std::vector<std::string>, CgipMask*, std::vector<std::string>) = 0;

		inline std::vector<CgipVolume*> getVolumePtr() { return m_volume_vector; }
		/**
		 *	Name:	setVolumeInfo
		 *	Params:
		 *		- vol_info:		VolumeInfo
		 *				fundamental information of volume data. 
		 *	Desc:	set fundamental information of volume data.
		 */
		inline void setVolumeInfo(VolumeInfo vol_info) { m_volume_info = vol_info; }
		inline VolumeInfo getVolumeInfo() { return m_volume_info; }
		inline int getChannel() { return m_channel; }

	protected:
		std::string m_ext;
		int m_channel;

		// Save info as cgipvolume
		std::vector<CgipVolume*> m_volume_vector;
		VolumeInfo m_volume_info;

		std::vector<char> _decodeBase64(const char* input_base64, int input_len);
	};

	CGIPIO_DLL CgipFileIO* getIO(const char* c_ext);
	CGIPIO_DLL CgipFileIO* getIO(std::vector<std::string> files);

	/**
	 *	Name:	getFileList
	 *	Params:
	 *		- dir_path:		std::string
	 *				string of directory path
	 *	Desc:	Get file paths in dir_path.
	 */
	CGIPIO_DLL std::vector<std::string> getFileList(std::string dir_path);

	/**
	 *	Name:	checkDirUnitary
	 *	Params:
	 *		- dir_path:		const char*
	 *				string of directory path
	 *	Desc:	Check the directory has only single file extensions.
	 */
	CGIPIO_DLL std::string checkDirUnitary(const char* dir_path);
	/**
	 *	Name:	checkFilesUnitary
	 *	Params:
	 *		- files:		std::vector<std::string> files
	 *				string list of file paths
	 *	Desc:	Check the file paths have all only single file extensions.
	 */
	CGIPIO_DLL std::string checkFilesUnitary(std::vector<std::string> files);

	/**
	 *	Name:	checkFileExistance
	 *	Params:
	 *		- file_path:		const std::string&
	 *				string of file path
	 *	Desc:	Check the existance of file
	 *			Return the boolean variable to represent the existance.
	 *			'True' means the file already exists, 'False' means it doesn't.
	 */
	CGIPIO_DLL inline bool checkFileExistance(const std::string& file_path) {
		struct stat buffer;
		return (stat(file_path.c_str(), &buffer) == 0);
	}
}
