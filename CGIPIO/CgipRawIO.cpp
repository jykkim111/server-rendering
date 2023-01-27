#include "CgipRawIO.h"

#include <Windows.h>
#include <fstream>

namespace cgip {
	CgipRawIO::CgipRawIO() :CgipFileIO() {
		m_ext = "raw";
	}

	// TODO: read raw vol
	void CgipRawIO::readVolume(std::vector<std::string> files) {
		if (files.size() == 1) {
			_readRawVolume(files[0]);
		}
		else {
			// TODO: raw image series
		}
	}

	void CgipRawIO::readPlane(std::string file) {

	}

	void CgipRawIO::saveVolume(CgipVolume& vol) {
		CgipShort** data = vol.getRawPtr();

		// TODO: revise dst filename
		std::vector<std::string> files;
		files.push_back(std::string("test.raw"));
		_saveRawVolume(data, vol.getDepth(), vol.getHeight(), vol.getWidth(), files);
	}

	void CgipRawIO::readVolume(std::string vol_dir, std::string vol_meta_dir) {
		// read volume info
		json metaData;
		std::ifstream i(vol_meta_dir);
		i >> metaData;

		m_volume_info.width = std::stoi(metaData["tag_Rows"].dump());
		m_volume_info.height = std::stoi(metaData["tag_Columns"].dump());
		m_volume_info.depth = metaData["FileObjList"].size();

		m_volume_info.rescale_intercept = std::stof(metaData["tag_rescale_intercept"].dump());
		m_volume_info.rescale_slope = std::stof(metaData["tag_rescale_slope"].dump());
		m_volume_info.samples_per_pixel = std::stoi(metaData["tag_samples_per_pixel"].dump());
		m_volume_info.spacing_x = std::stof(metaData["tag_pixel_spacingRow"].dump());
		m_volume_info.spacing_y = std::stof(metaData["tag_pixel_spacingCol"].dump());
		m_volume_info.spacing_z = std::stof(metaData["tag_slice_thickness"].dump());

		m_volume_info.default_window_center = std::stoi(metaData["tag_window_center"].dump());
		m_volume_info.default_window_width = std::stoi(metaData["tag_window_width"].dump());
		m_volume_info.modality = metaData["tag_modality"].dump();

		// read volume
		std::ifstream inFile2;
		inFile2.open(vol_dir, std::ios::in | std::ios::binary);
		if (!inFile2.is_open())
		{
			throw CgipException("Cannot load file: " + vol_dir);
			return;
		}

		const int volume_size = m_volume_info.width * m_volume_info.height * m_volume_info.depth;
		int data_type = sizeof(int16_t);
		std::unique_ptr<unsigned char> readBuffer(new unsigned char[volume_size * data_type]);
		std::memset(readBuffer.get(), 0, sizeof(unsigned char) * volume_size * data_type);

		inFile2.read(reinterpret_cast<char*>(readBuffer.get()), volume_size * data_type);

		CgipVolume* new_volume = new CgipVolume(m_volume_info.width, m_volume_info.height, m_volume_info.depth);
		new_volume->setSpacingX(m_volume_info.spacing_x);
		new_volume->setSpacingY(m_volume_info.spacing_y);
		new_volume->setSpacingZ(m_volume_info.spacing_z);

		for (int i = 0; i < m_volume_info.depth * m_volume_info.height * m_volume_info.width * data_type; i += data_type) {
			int z = (i / data_type) / (m_volume_info.height * m_volume_info.width);
			int y = ((i / data_type) % (m_volume_info.height * m_volume_info.width)) / m_volume_info.width;
			int x = ((i / data_type) % (m_volume_info.height * m_volume_info.width)) % m_volume_info.width;

			// Todo: consider byte order
			int16_t val = static_cast<unsigned char>(reinterpret_cast<unsigned char*>(readBuffer.get())[i]);
			val = val << 8;
			val += static_cast<unsigned char>(reinterpret_cast<unsigned char*>(readBuffer.get())[i + 1]);
			val = (short)_byteswap_ushort(val);

			val = m_volume_info.rescale_slope * val + m_volume_info.rescale_intercept;
			new_volume->setVoxelValue(x, y, z, val);
		}

		m_volume_vector.clear();
		m_volume_vector.push_back(new_volume);
		inFile2.close();
	}

	CgipMask CgipRawIO::readMask(std::string file, CgipMask* old_mask, std::vector<std::string> vol_uid_list) {
		CgipMask new_mask(1, 1, 1);
		return std::move(new_mask);
	}

	// TODO:: Implement this
	CgipMask CgipRawIO::readMask(std::vector<std::string> files, CgipMask* old_mask, std::vector<std::string> vol_uid_list) {
		cgip::CgipMask temp = cgip::CgipMask(1, 1, 1);
		return std::move(temp);
	}

	void CgipRawIO::readMask(std::string dir_path, CgipMask* mask) {
		std::ifstream inFile2;
		inFile2.open(dir_path, std::ios::in | std::ios::binary);
		if (!inFile2.is_open())
		{
			throw CgipException("Cannot load file: " + dir_path);
			return;
		}

		const int mask_size = mask->getWidth() * mask->getHeight() * mask->getDepth();
		int size_type = sizeof(unsigned char);
		std::unique_ptr<unsigned char> readBuffer(new unsigned char[mask_size * size_type]);
		std::memset(readBuffer.get(), 0, sizeof(char) * mask_size * size_type);

		inFile2.read(reinterpret_cast<char*>(readBuffer.get()), mask_size * size_type);

		int idx = 0;
		for (int z = 0; z < mask->getDepth(); z++)
			for (int xy = 0; xy < mask->getWidth() * mask->getHeight(); xy++)
			{
				int y = xy / mask->getWidth();
				int x = xy % mask->getWidth();
				unsigned char val = static_cast<unsigned char>(reinterpret_cast<unsigned char*>(readBuffer.get())[idx++]);
				mask->setVoxelValue(x, y, z, static_cast<short>(val));
			}
		inFile2.close();
	}

	SegMask* CgipRawIO::readMask(std::string dir_path) {
		std::ifstream* pFile = new std::ifstream;
		SegMask* mask = nullptr;

		pFile->open(dir_path, std::ios::in | std::ios::binary);
		if (!pFile->is_open())
		{
			printf("Cannot load file: %s\n", dir_path.c_str());
			throw CgipException("Cannot load file: " + dir_path);
			return mask;
		}

		try
		{
			DWORD dwVersion;
			pFile->read((char*)&dwVersion, sizeof(DWORD));
			//*pFile >> dwVersion;

			if (dwVersion >= 0x30000001)
			{
				// get dimension
				int nWidth, nHeight, nSlices, nBufferWidth;
				pFile->read((char*)&nWidth, sizeof(int));
				pFile->read((char*)&nHeight, sizeof(int));
				pFile->read((char*)&nSlices, sizeof(int));
				pFile->read((char*)&nBufferWidth, sizeof(int));
				//*pFile >> nWidth >> nHeight >> nSlices >> nBufferWidth;

				//printf("in: %d, %d, %d, %d", nWidth, nHeight, nSlices, nBufferWidth);

				if (nWidth > 0 && nHeight > 0 && nSlices > 0 && nBufferWidth > 0)
				{
					mask = new SegMask(nWidth, nHeight, nSlices, nBufferWidth);

					const int slice_sz = nBufferWidth * nHeight;
					bool bIncludeData = true;

					if (dwVersion >= 0x30000002) {
						//*pFile >> bIncludeData;
						pFile->read((char*)&bIncludeData, sizeof(bool));
					}

					if (bIncludeData)
					{
						for (int i = 0; i < nSlices; i++)
						{
							int size_type = sizeof(unsigned char);
							std::unique_ptr<unsigned char> readBuffer(new unsigned char[slice_sz * size_type]);
							std::memset(readBuffer.get(), 0, sizeof(char) * slice_sz * size_type);

							pFile->read(reinterpret_cast<char*>(readBuffer.get()), size_type * slice_sz);

							for (int xy = 0; xy < slice_sz; xy++)
							{
								unsigned char val = static_cast<unsigned char>(reinterpret_cast<unsigned char*>(readBuffer.get())[xy]);
								mask->m_mask[nSlices - (i + 1)][xy] = static_cast<BYTE>(val);
							}
						}
					}
				}
			}
		}
		catch(std::exception const& e)
		{
			if (mask)
			{
				delete mask;
				mask = NULL;
			}
		}

		pFile->close();
		return mask;
	}

	bool CgipRawIO::saveMask(std::string dir_path, SegMask mask) {
		std::ofstream *pFile = new std::ofstream(dir_path, std::ios::binary | std::ios::out);
		if (!pFile->is_open()) {
			throw CgipException("Cannot save file: " + dir_path);
			return false;
		}

		bool bOk = true;
		bool bIncludeData = true;

		try
		{
			// version first
			const DWORD dwVersion = 0x30000002;
			//*pFile << dwVersion;
			pFile->write(reinterpret_cast<const char*>(&dwVersion), sizeof(DWORD));

			int mWidth = mask.getWidth();
			int mHeight = mask.getHeight();
			int mDepth = mask.getDepth();
			int mBufferWidth = mask.m_nBufferWidth;

			// dimension
			pFile->write(reinterpret_cast<char*>(&mWidth), sizeof(int));
			pFile->write(reinterpret_cast<char*>(&mHeight), sizeof(int));
			pFile->write(reinterpret_cast<char*>(&mDepth), sizeof(int));
			pFile->write(reinterpret_cast<char*>(&mBufferWidth), sizeof(int));
			//*pFile << mask->m_nWidth << mask->m_nHeight << mask->m_nSlices << mask->m_nBufferWidth;

			//printf("out: %d, %d, %d, %d", mask->m_nWidth, mask->m_nHeight, mask->m_nSlices, mask->m_nBufferWidth);

			// write buffer
			const int slice_sz = mask.m_nBufferWidth * mask.getHeight();

			//*pFile << bIncludeData;
			pFile->write(reinterpret_cast<char*>(&bIncludeData), sizeof(bool));

			if (bIncludeData)
			{
				for (int i = 0; i < mask.getDepth(); i++) {
					unsigned char* loaded_mask = new unsigned char[slice_sz];

					for (int xy = 0; xy < slice_sz; xy++)
					{
						loaded_mask[xy] = static_cast<unsigned char>(mask.m_mask[mask.getDepth() - (i+1)][xy]);
					}

					pFile->write(reinterpret_cast<char*>(loaded_mask), sizeof(unsigned char) * slice_sz);
				}
			}
		}
		catch(std::exception const& e)
		{
			bOk = false;
		}
		pFile->close();

		return bOk;
	}

	void CgipRawIO::saveVolume(CgipMask& mask, std::vector<std::string> file_list) {
		CgipShort** data = mask.getRawArray();

		_saveRawVolume(data, mask.getDepth(), mask.getHeight(), mask.getWidth(), file_list);
	}
	void CgipRawIO::_saveRawVolume(CgipShort** data, int depth, int height, int width, std::vector<std::string> file_list) {
		assert (file_list.size() == 1);

		std::ofstream ofs;
		ofs.open(file_list[0], std::ios::out | std::ios::binary);

		for (int i = 0; i < depth; i++) {
			ofs.write(reinterpret_cast<char*>(data[i]), sizeof(short)*height*width);
		}
		ofs.close();
	}

	void CgipRawIO::_readRawVolume(std::string file_path) {
		// Raw data will be assume all spacing is 1mm.
		m_volume_info.spacing_x = 1;
		m_volume_info.spacing_y = 1;
		m_volume_info.spacing_z = 1;

		std::ifstream ifs;
		ifs.open(file_path, std::ios::in | std::ios::binary);
		if (!ifs.is_open()) {
			throw CgipException("Cannot load file: " + file_path);
		}

		// TODO: compare data size and inputed volume dimension.

		int cnt_voxels = m_volume_info.width * m_volume_info.height * m_volume_info.depth;
		int data_type = sizeof(int16_t); // Todo: allow another data type

		std::unique_ptr<unsigned char> buf(new unsigned char[data_type * cnt_voxels]);
		std::memset(buf.get(), 0, sizeof(unsigned char) * data_type * cnt_voxels);

		ifs.read(reinterpret_cast<char*>(buf.get()), data_type * cnt_voxels);

		// Allocate memory for volume data
		CgipVolume* new_volume = new CgipVolume(m_volume_info.width, m_volume_info.height, m_volume_info.depth);
		new_volume->setSpacingX(m_volume_info.spacing_x);
		new_volume->setSpacingY(m_volume_info.spacing_y);
		new_volume->setSpacingZ(m_volume_info.spacing_z);

		for (int i = 0; i < m_volume_info.depth * m_volume_info.height * m_volume_info.width * data_type; i+=data_type) {
			int z = (i/data_type) / (m_volume_info.height * m_volume_info.width);
			int y = ((i/data_type) % (m_volume_info.height * m_volume_info.width)) / m_volume_info.width;
			int x = ((i/data_type) % (m_volume_info.height * m_volume_info.width)) % m_volume_info.width;

			// Todo: consider byte order
			int16_t val = static_cast<unsigned char>(reinterpret_cast<unsigned char*>(buf.get())[i]);
			val = val << 8;
			val += static_cast<unsigned char>(reinterpret_cast<unsigned char*>(buf.get())[i+1]);
			val = (short)_byteswap_ushort(val);

			val = m_volume_info.rescale_slope * val + m_volume_info.rescale_intercept;
			new_volume->setVoxelValue(x, y, z, val);
		}

		m_volume_vector.clear();
		m_volume_vector.push_back(new_volume);

		ifs.close();
	}

}