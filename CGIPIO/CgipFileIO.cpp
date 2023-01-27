#include "CgipFileIO.h"
#include "CgipDicomIO.h"
#include "CgipRawIO.h"
#include "CgipImageIO.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <string>

namespace cgip {
	CgipFileIO::CgipFileIO() {
	}

	std::vector<char> CgipFileIO::_decodeBase64(const char* input_base64, int input_len) {
		std::vector<char> result;
		int result_len = 0;
		int phase = 0;
		int d, prev_d = 0;
		char c;

		for (int i = 0; i < input_len; i++) {
			int base64_idx = (int)(input_base64[i]);
			if (base64_idx < 0 || 256 <= base64_idx)
				throw CgipException("FILE IO Err: This is not base64 encoded data");

			d = BASE64_DECODE_TABLE[base64_idx];
			if (d != -1) {
				switch (phase) {
				case 0:
					phase++;
					break;
				case 1:
					c = ((prev_d << 2) | ((d & 0x30) >> 4));
					if (result_len < input_len) {
						result.push_back(c);
						result_len++;
					}

					phase++;
					break;
				case 2:
					c = (((prev_d & 0xf) << 4) | ((d & 0x3c) >> 2));
					if (result_len < input_len) {
						result.push_back(c);
						result_len++;
					}

					phase++;
					break;
				case 3:
					c = (((prev_d & 0x03) << 6) | d);
					if (result_len < input_len) {
						result.push_back(c);
						result_len++;
					}

					phase = 0;
				}

				prev_d = d;
			}
		}

		return std::move(result);
	}

	CgipFileIO* getIO(const char* c_ext) {
		std::string ext(c_ext);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		CgipFileIO* io;

		if (ext == "dcm") {
			io = new CgipDicomIO();
			return io;
		}
		else if(ext == "raw"){
			io = new CgipRawIO();
			return io;
		}
		else if(ext == "png"){
			io = new CgipImageIO(ext);
			return io;
		}
		else if (ext == "jpeg") {
			io = new CgipImageIO(ext);
			return io;
		}
		else {
			throw CgipException("Not support ext");
			// TODO:: Error Handling
			//Debug
			//std::cout << "Not support ext" << std::endl;
			//io = new CgipDicomIO();	//TODO:: modify default ext
			//return io;
		}
	}

	CgipFileIO* getIO(std::vector<std::string> files) {
		std::string ext = checkFilesUnitary(files);

		return getIO(ext.c_str());
	}

	/**
	 *	Name:		getFileList
	 *	Created:	2020-09-15
	 *	Modified:	2020-12-21
	 *  Author:		Sanguk Park
	 *	Params:
	 *		- dir_path:		std::string
	 *			string of directory path
	 *	Desc:		Get file paths in dir_path.
	 */
	std::vector<std::string> getFileList(std::string dir_path) {

#ifdef _WIN32
		std::string dir_all_path = dir_path + "/*";
		wchar_t wc_dir_path[FILENAME_MAX];
		MultiByteToWideChar(CP_ACP, 0, dir_all_path.c_str(), -1, wc_dir_path, FILENAME_MAX);
		LPCWSTR pwc_dir_path = wc_dir_path;

		std::vector<std::string> files;
		WIN32_FIND_DATA filename;
		HANDLE h_find = FindFirstFile(pwc_dir_path, &filename);

		if (h_find != INVALID_HANDLE_VALUE) {
			do {
				char filename_char[FILENAME_MAX];
				WideCharToMultiByte(CP_ACP, 0, filename.cFileName, -1,
					filename_char, FILENAME_MAX, NULL, NULL);

				// Skip for cur dir and parent dir
				if (std::string(filename_char) == "."
					|| std::string(filename_char) == "..") {
					continue;
				}

				files.push_back(dir_path
					+ "/"
					+ std::string(filename_char));
			} while (FindNextFile(h_find, &filename));
			FindClose(h_find);
		}

		return files;
#endif
	}

	/**
	 *	Name:		checkDirUnitary
	 *	Created:	2020-09-15
	 *	Modified:	2020-09-15
	 *  Author:		Sanguk Park
	 *	Params:
	 *		- dir_path:		const char*
	 *			string of directory path
	 *	Desc:		Check the directory has only single file extensions.
	 *				Return the LOWERCASE string of file extension. 
	 *				If there is multiple extensions, return the null string.	 
	 */
	std::string checkDirUnitary(const char* dir_path) {

		std::vector<std::string> files = getFileList(dir_path);

		return checkFilesUnitary(files);
	}

	/**
	 *	Name:		checkFilesUnitary
	 *	Created:	2020-09-15
	 *	Modified:	2021-01-07
	 *  Author:		Sanguk Park
	 *	Params:
	 *		- files:		std::vector<std::string> files
	 *			string list of file paths
	 *	Desc:		Check the file paths have all only single file extensions.
	 *				Return the LOWERCASE string of file extension.
	 *				If there is multiple extensions, return the null string.
	 */
	std::string checkFilesUnitary(std::vector<std::string> files) {
		// Return null when empty dir
		if (files.size() == 0) return std::string();

		std::string ext = files[0];
		size_t idx_ext = ext.find_last_of('.');
		size_t len_ext = ext.length();

		// There is no extension
		if (idx_ext == std::string::npos) {
			return std::string();
		}

		ext = ext.substr(idx_ext + 1, len_ext - idx_ext);

		for (auto file : files) {
			std::string other_ext = file;
			idx_ext = other_ext.find_last_of('.');
			len_ext = other_ext.length();

			// There is no extension
			if (idx_ext == std::string::npos) {
				return std::string();
			}

			other_ext = other_ext.substr(idx_ext + 1, len_ext - idx_ext);

			// Not correspondent ext
			if (other_ext != ext) {
				return std::string();
			}
		}

		// Transform to lowercase
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext;
	}

}