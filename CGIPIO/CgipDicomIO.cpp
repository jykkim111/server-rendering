#include "CgipDicomIO.h"

#include "../CgipException/CgipException.h"

// GDCM 3.0.0 Libraries
#include <gdcmScanner.h>
#include <gdcmImageReader.h>
#include <gdcmPixmap.h>
#include <gdcmAttribute.h>
#include <gdcmDataElement.h>
#include <gdcmSequenceOfItems.h>
#include <gdcmImageChangeTransferSyntax.h>
#include <gdcmStringFilter.h>

#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>

namespace cgip {
	CgipDicomIO::CgipDicomIO() : CgipFileIO() {
		m_ext = "dcm";
	}

	void CgipDicomIO::readVolume(std::vector<std::string> files) {
		readDicom(files);
	}

	void CgipDicomIO::readPlane(std::string file) {
		// Create a temporal vector
		std::vector<std::string> files;
		files.push_back(file);

		readDicom(files);
	}

	// TODO: Save to dicom image file
	void CgipDicomIO::saveVolume(CgipVolume& vol) {

	}
	void CgipDicomIO::saveVolume(CgipMask& mask, std::vector<std::string> file_list) {

	}

	CgipMask CgipDicomIO::readMask(std::string file, CgipMask* old_mask, std::vector<std::string> vol_uid_list) {
		return std::move(*old_mask);
	}

	// TODO:: Implement this
	CgipMask CgipDicomIO::readMask(std::vector<std::string> files, CgipMask* old_mask, std::vector<std::string> vol_uid_list) {
		cgip::CgipMask temp = cgip::CgipMask(1, 1, 1);
		return std::move(temp);
	}


	bool CgipDicomIO::validateVolume(std::vector<std::string> files, std::string &errorMsg) {

		std::cout << "validating volume... " << std::endl;
		int row;
		int column;
		float slice_spacing; 
		const double* orientation;


		for (int i = 1; i < files.size()-1; i++) {
			gdcm::Reader reader;
			reader.SetFileName(files[i].c_str());

			if (!reader.Read()) {
				throw CgipException("Error: failed dicom loading!");
			}

			const gdcm::File& f = reader.GetFile();
			const gdcm::DataSet& ds = f.GetDataSet();

			gdcm::Tag tag_row(0x0028, 0x0010);
			gdcm::Tag tag_column(0x0028, 0x0011);
			gdcm::Tag tag_image_orientation(0x0020, 0x0037);
			gdcm::Tag tag_spacing_between_slices(0x0018, 0x0050);


			if (ds.FindDataElement(tag_row)) {
				gdcm::Attribute<0x0028, 0x0010> at_row;
				at_row.Set(ds);
				if (i == 1) {
					row = at_row.GetValue();
				}
				int current_row_value = at_row.GetValue();

				if (row != current_row_value) {
					errorMsg = "slices don't have equal number of rows";
					return false;
				}
			}

			if (ds.FindDataElement(tag_column)) {
				gdcm::Attribute<0x0028, 0x0011> at_column;
				at_column.Set(ds);
				if (i == 1) {
					column = at_column.GetValue();
				}
				int current_column_value = at_column.GetValue();

				if (column != current_column_value) {
					errorMsg = "Slices don't have equal number of columns";
					return false;
				}
			}

			if (ds.FindDataElement(tag_image_orientation)) {
				gdcm::Attribute<0x0020, 0x0037> at_image_orientation;
				at_image_orientation.Set(ds);
				if (i == 1) {
					orientation = at_image_orientation.GetValues();
				}
				const double* c = at_image_orientation.GetValues();

				for (int j = 0; j < 6; j++) {
					if (orientation[j] != c[j]) {
						errorMsg = "Slices don't have the same image orientation";
						return false;
					}
				}
			}

			if (ds.FindDataElement(tag_spacing_between_slices)) {
				gdcm::Attribute<0x0028, 0x0011> at_slice_spacing;
				at_slice_spacing.Set(ds);
				if (i == 1) {
					slice_spacing = at_slice_spacing.GetValue();
				}
				float current_slice_spacing_value = at_slice_spacing.GetValue();

				if (slice_spacing != current_slice_spacing_value) {
					errorMsg = "Slices don't have the same slice spacing values";
					return false;
				}
			}

		}
		
		return true;
	}



	void CgipDicomIO::readDicom(std::vector<std::string> files) {
		readDicomHeader(files[files.size() - 1].c_str());

		if (m_volume_info.modality == "US" || m_volume_info.modality == "ES") {
			//_readUltraSound(files);
		}
		else
			_readDicom(files);
	}

	/**
		 *	Name:		getImageNumber
		 *	Created:	2020-12-21
		 *	Modified:	2020-12-21
		 *	Author:		Sanguk Park
		 *	Params:
		 *		- file:	std::string
		 *			path of a dicom file
		 *	Desc:		return image number tag(0020, 0013) value
		 *				Image number tag may be empty if it is unknown.
		 */
	int CgipDicomIO::getImageNumber(std::string file){
		gdcm::Reader reader;
		reader.SetFileName(file.c_str());

		if (!reader.Read()) {
			throw CgipException("Error: failed dicom loading!");
		}

		const gdcm::File& f = reader.GetFile();
		const gdcm::DataSet& ds = f.GetDataSet();

		gdcm::Tag tag_image_number = gdcm::Tag(0x0020, 0x0013);

		if (ds.FindDataElement(tag_image_number)) {
			auto val = ds.GetDataElement(tag_image_number);

			if (val.IsEmpty()) {
				throw CgipUnknownException(
					"Dicom Error: Unknown value <0020, 0013> Image Number");
			}
			else {
				gdcm::Attribute<0x0020, 0x0013> at_image_number;
				at_image_number.Set(ds);

				return at_image_number.GetValue();
			}
		}
		else {
			throw CgipUnexistException(
				getTagNotFoundMsg(tag_image_number.PrintAsPipeSeparatedString()));
		}
	}



	void CgipDicomIO::_readDicom(std::vector<std::string> files) {
		if (files.size() <= 1) {
			//throw CgipException("Dicom Err: Need two or more slices");
			m_volume_info.spacing_z = 1;
		}
		else {
			computeSpacingZ(files[0].c_str(), files[1].c_str());

			gdcm::ImageReader reader1, reader2, reader3;
			reader1.SetFileName(files[0].c_str()); // frame 0
			reader2.SetFileName(files[1].c_str()); // frame 1
			reader3.SetFileName(files[files.size() - 1].c_str()); // last frame
			if (!reader1.Read() || !reader2.Read() || !reader3.Read()) {
				throw std::runtime_error("ERR!");
			}

			CgipFloat spacing_z;
			gdcm::Tag tag_image_position(0x0020, 0x0032);
			gdcm::File& r_f1 = reader1.GetFile();
			gdcm::File& r_f2 = reader2.GetFile();
			gdcm::File& r_f3 = reader3.GetFile();
			gdcm::DataSet& r_ds1 = r_f1.GetDataSet();
			gdcm::DataSet& r_ds2 = r_f2.GetDataSet();
			gdcm::DataSet& r_ds3 = r_f3.GetDataSet();

			if (r_ds1.FindDataElement(tag_image_position) && r_ds2.FindDataElement(tag_image_position) && r_ds3.FindDataElement(tag_image_position)) {
				gdcm::Attribute<0x0020, 0x0032> at_image_position_patient;
				at_image_position_patient.Set(r_ds1);
				m_volume_info.m_FirstImg_x = at_image_position_patient.GetValue(0);
				m_volume_info.m_FirstImg_y = at_image_position_patient.GetValue(1);
				m_volume_info.m_FirstImg_z = at_image_position_patient.GetValue(2);
				at_image_position_patient.Set(r_ds2);
				m_volume_info.m_SecondImg_x = at_image_position_patient.GetValue(0);
				m_volume_info.m_SecondImg_y = at_image_position_patient.GetValue(1);
				m_volume_info.m_SecondImg_z = at_image_position_patient.GetValue(2);
				at_image_position_patient.Set(r_ds3);
				m_volume_info.m_LastImg_x = at_image_position_patient.GetValue(0);
				m_volume_info.m_LastImg_y = at_image_position_patient.GetValue(1);
				m_volume_info.m_LastImg_z = at_image_position_patient.GetValue(2);

			}
		}

		// Debug
		if (m_volume_info.spacing_z < 1e-3) {
			m_volume_info.spacing_z = 1;
			//throw CgipException("Dicom Err: Invalid spacing z");
		}

		// Create a volume
		m_volume_info.depth = files.size();
		m_volume_vector.clear();
		for (int i = 0; i < m_volume_info.samples_per_pixel; i++) {
			CgipVolume* new_volume = new CgipVolume(m_volume_info.width, m_volume_info.height, m_volume_info.depth);
			new_volume->setSpacingX(m_volume_info.spacing_x);
			new_volume->setSpacingY(m_volume_info.spacing_y);
			new_volume->setSpacingZ(m_volume_info.spacing_z);

			m_volume_vector.push_back(new_volume);
		}


		int slice_cnt = 0;
		//m_uid_list.clear();
		for (auto filename : files) {

			// For Debug
			gdcm::ImageReader reader;
			reader.SetFileName(filename.c_str());

			if (!reader.Read()) {
				throw std::runtime_error("ERR!");
			}

			const gdcm::File &f = reader.GetFile();
			const gdcm::DataSet &ds = f.GetDataSet();

			/*
			gdcm::Tag tag_series_uid(0x0020, 0x000E);

			if (ds.FindDataElement(tag_series_uid)) {
				auto series_uid = ds.GetDataElement(tag_series_uid).GetByteValue();
				int series_uid_len = series_uid->GetLength();
				std::string series_uid_str(series_uid->GetPointer());
				series_uid_str = series_uid_str.substr(0, series_uid_len);

				if (slice_cnt == 0)
					m_volume_info.series_uid = series_uid_str;
				else if (m_volume_info.series_uid != series_uid_str)
					throw CgipException("Dicom Err: Slices have different series uid");
			}
			else {
				throw CgipException(getTagNotFoundMsg(tag_series_uid.PrintAsPipeSeparatedString()));
			}

			gdcm::Tag tag_uid(0x0008, 0x0018);

			if (ds.FindDataElement(tag_uid)) {
				auto uid = ds.GetDataElement(tag_uid).GetByteValue();
				int uid_len = uid->GetLength();
				std::string uid_str(uid->GetPointer());
				uid_str = uid_str.substr(0, uid_len);

				m_uid_list.push_back(uid_str);
			}
			else {
				throw CgipException(getTagNotFoundMsg(tag_uid.PrintAsPipeSeparatedString()));
			}
			*/

			// Set windowing parameter depends on center axial plane of volume.
			if (slice_cnt == (files.size() - 1) / 2) {
				gdcm::Tag tag_window_center(0x0028, 0x1050);
				gdcm::Tag tag_window_width(0x0028, 0x1051);

				if (ds.FindDataElement(tag_window_center) && ds.FindDataElement(tag_window_width)) {
					gdcm::Attribute<0x0028, 0x1050> at_window_center;
					gdcm::Attribute<0x0028, 0x1051> at_window_width;
					at_window_center.Set(ds);
					at_window_width.Set(ds);
					m_volume_info.default_window_center = at_window_center.GetValue();
					m_volume_info.default_window_width = at_window_width.GetValue();
				}
				else {
					m_volume_info.default_window_center = 0;
					m_volume_info.default_window_width = 0;
				}
			}

			// Support for compressed dicom
			gdcm::ImageChangeTransferSyntax converter = gdcm::ImageChangeTransferSyntax();
			converter.SetTransferSyntax(gdcm::TransferSyntax(gdcm::TransferSyntax::ImplicitVRLittleEndian));
			converter.SetInput(reader.GetImage());
			if (!converter.Change()) {
				throw cgip::CgipException("Change Error!");
			}

			const gdcm::Pixmap &image = converter.GetOutput();
			std::vector<char> buf;
			buf.resize(image.GetBufferLength());
			image.GetBuffer(buf.data());
			short *buf16 = (short*)(buf.data()); // TODO: consider "Bits Allocated(0028, 0100)"

			//std::cout << " m_volume_info.width: " << m_volume_info.width << std::endl;
			//std::cout << " m_volume_info.height: " << m_volume_info.height << std::endl;
			//std::cout << " m_volume_info.samples_per_pixel: " << m_volume_info.samples_per_pixel << std::endl;


			// Copy to array
#pragma omp parallel for
			for (int i = 0; i < m_volume_info.height * m_volume_info.width * m_volume_info.samples_per_pixel; i++) {
				int y = i / (m_volume_info.width * m_volume_info.samples_per_pixel);
				int x = (i % (m_volume_info.width * m_volume_info.samples_per_pixel)) / m_volume_info.samples_per_pixel;
				int c = (i % (m_volume_info.width * m_volume_info.samples_per_pixel)) % m_volume_info.samples_per_pixel;
				short val = m_volume_info.rescale_slope * buf16[i] + m_volume_info.rescale_intercept;
				m_volume_vector[c]->setVoxelValue(x, y, slice_cnt, val);
			}

			slice_cnt++;
		}
	}

	/*
	void CgipDicomIO::_readUltraSound(std::vector<std::string> files) {
		if (files.size() != 1)
			throw cgip::CgipException("Trying to read more than one Ultrasound Files");

		m_volume_info.spacing_z = 1;

		// Create a volume
		m_volume_vector.clear();
		for (int i = 0; i < m_volume_info.samples_per_pixel; i++) {
			CgipVolume* new_volume = new CgipVolume(m_volume_info.width, m_volume_info.height, m_volume_info.depth);
			new_volume->setSpacingX(m_volume_info.spacing_x);
			new_volume->setSpacingY(m_volume_info.spacing_y);
			new_volume->setSpacingZ(m_volume_info.spacing_z);

			m_volume_vector.push_back(new_volume);
		}

		gdcm::ImageReader reader;
		reader.SetFileName(files[0].c_str());
		if (!reader.Read()) {
			throw std::runtime_error("ERR!");
		}

		int slice_cnt = 0;
		m_uid_list.clear();
		const gdcm::File &f = reader.GetFile();
		const gdcm::DataSet &ds = f.GetDataSet();

		gdcm::Tag tag_series_uid(0x0020, 0x000E);

		if (ds.FindDataElement(tag_series_uid)) {
			auto series_uid = ds.GetDataElement(tag_series_uid).GetByteValue();
			int series_uid_len = series_uid->GetLength();
			std::string series_uid_str(series_uid->GetPointer());
			series_uid_str = series_uid_str.substr(0, series_uid_len);

			if (slice_cnt == 0)
				m_volume_info.series_uid = series_uid_str;
			else if (m_volume_info.series_uid != series_uid_str)
				throw CgipException("Dicom Err: Slices have different series uid");
		}
		else {
			throw CgipException(getTagNotFoundMsg(tag_series_uid.PrintAsPipeSeparatedString()));
		}

		gdcm::Tag tag_uid(0x0008, 0x0018);

		if (ds.FindDataElement(tag_uid)) {
			auto uid = ds.GetDataElement(tag_uid).GetByteValue();
			int uid_len = uid->GetLength();
			std::string uid_str(uid->GetPointer());
			uid_str = uid_str.substr(0, uid_len);

			m_uid_list.push_back(uid_str);
		}
		else {
			throw CgipException(getTagNotFoundMsg(tag_uid.PrintAsPipeSeparatedString()));
		}

		// Support for compressed dicom
		gdcm::ImageChangeTransferSyntax converter = gdcm::ImageChangeTransferSyntax();
		converter.SetTransferSyntax(gdcm::TransferSyntax(gdcm::TransferSyntax::ImplicitVRLittleEndian));
		converter.SetInput(reader.GetImage());
		if (!converter.Change()) {
			throw cgip::CgipException("Change Error!");
		}

		const gdcm::Pixmap &image = converter.GetOutput();

		std::vector<char> buf;
		buf.resize(image.GetBufferLength());
		image.GetBuffer(buf.data());
		unsigned char *buf8 = (unsigned char*)(buf.data()); // TODO: consider "Bits Allocated(0028, 0100)"

		// Copy to array
#pragma omp parallel for
		for (int i = 0; i < m_volume_info.depth * m_volume_info.height * m_volume_info.width * m_volume_info.samples_per_pixel; i++) {
			int z0 = i / (m_volume_info.height * m_volume_info.width * m_volume_info.samples_per_pixel);
			int z1 = i % (m_volume_info.height * m_volume_info.width * m_volume_info.samples_per_pixel);
			int y0 = z1 / (m_volume_info.width * m_volume_info.samples_per_pixel);
			int y1 = z1 % (m_volume_info.width * m_volume_info.samples_per_pixel);
			int x0 = y1 / m_volume_info.samples_per_pixel;
			int c0 = y1 % m_volume_info.samples_per_pixel;
			short val = m_volume_info.rescale_slope * buf8[i] + m_volume_info.rescale_intercept;
			m_volume_vector[c0]->setVoxelValue(x0, y0, z0, val);
		}
	}

	// Todo: Revise this
	void CgipDicomIO::_setPhotometry(std::string ts_uid, int samples_per_pixel) {
		m_volume_info.photometry = "Monochrome";

		// Below is not confident
		if (samples_per_pixel == 3) {
			m_volume_info.photometry = "RGB";
		}

		if (ts_uid == "1.2.840.10008.1.2.4.50" && samples_per_pixel == 3) {
			m_volume_info.photometry = "YUV";
		}
	}
	*/

	void CgipDicomIO::readDicomHeader(const char* filename) {
		gdcm::ImageReader reader;
		reader.SetFileName(filename);

		if (!reader.Read()) {
			throw std::runtime_error("ERR!");
		}

		gdcm::Image &image = reader.GetImage();
		m_volume_info.width = image.GetDimension(0);
		m_volume_info.height = image.GetDimension(1);

		const gdcm::File &f = reader.GetFile();
		const gdcm::FileMetaInformation &header = f.GetHeader();
		const gdcm::DataSet &ds = f.GetDataSet();

		gdcm::Tag tag_modality(0x0008, 0x0060);

		if (ds.FindDataElement(tag_modality)) {
			gdcm::Attribute<0x0008, 0x0060> at_modality;
			at_modality.Set(ds);
			m_volume_info.modality = at_modality.GetValue();

			// For debug
			at_modality.Print(std::cout);
		}
		else {
			throw CgipException(getTagNotFoundMsg(tag_modality.PrintAsPipeSeparatedString()));
		}

		if (m_volume_info.modality == "US" || m_volume_info.modality == "ES") {
			gdcm::Tag tag_frame_num(0x0028, 0x0008);

			if (ds.FindDataElement(tag_frame_num)) {
				gdcm::Attribute<0x0028, 0x0008> at_frame_num;
				at_frame_num.Set(ds);
				m_volume_info.depth = at_frame_num.GetValue();

				// For debug
				at_frame_num.Print(std::cout);
			}
			else {
				m_volume_info.depth = 1;
			}
		}

		gdcm::Tag tag_samples_per_pixel(0x0028, 0x0002);

		if (ds.FindDataElement(tag_samples_per_pixel)) {
			gdcm::Attribute<0x0028, 0x0002> at_samples_per_pixel;
			at_samples_per_pixel.Set(ds);
			m_volume_info.samples_per_pixel = at_samples_per_pixel.GetValue();

			// For debug
			at_samples_per_pixel.Print(std::cout);
		}
		else {
			throw CgipException(getTagNotFoundMsg(tag_samples_per_pixel.PrintAsPipeSeparatedString()));
		}

		gdcm::Tag tag_pixel_spacing(0x0028, 0x0030);

		if (ds.FindDataElement(tag_pixel_spacing)) {
			gdcm::Attribute<0x0028, 0x0030> at_pixel_spacing;
			at_pixel_spacing.Set(ds);
			m_volume_info.spacing_x = at_pixel_spacing.GetValue(0);
			m_volume_info.spacing_y = at_pixel_spacing.GetValue(1);

			// For debug
			at_pixel_spacing.Print(std::cout);
		}
		else {
			//throw CgipException(tag_not_found_msg(tag_pixel_spacing.PrintAsPipeSeparatedString()));
			m_volume_info.spacing_x = 1;
			m_volume_info.spacing_y = 1;
		}

		gdcm::Tag tag_slice_thickness(0x0018, 0x0050);

		if (ds.FindDataElement(tag_slice_thickness)) {
			gdcm::Attribute<0x0018, 0x0050> at_slice_thickness;
			at_slice_thickness.Set(ds);
			m_volume_info.spacing_z = at_slice_thickness.GetValue();

			// For debug
			at_slice_thickness.Print(std::cout);
		}
		else {
			//throw CgipException(tag_not_found_msg(tag_slice_thickness.PrintAsPipeSeparatedString()));
			m_volume_info.spacing_z = 1;
		}

		gdcm::Tag tag_window_center(0x0028, 0x1050);
		gdcm::Tag tag_window_width(0x0028, 0x1051);

		if (ds.FindDataElement(tag_window_center) && ds.FindDataElement(tag_window_width)) {
			gdcm::Attribute<0x0028, 0x1050> at_window_center;
			gdcm::Attribute<0x0028, 0x1051> at_window_width;
			at_window_center.Set(ds);
			at_window_width.Set(ds);
			m_volume_info.default_window_center = at_window_center.GetValue();
			m_volume_info.default_window_width = at_window_width.GetValue();

			// For debug
			at_window_center.Print(std::cout);
			at_window_width.Print(std::cout);
		}
		else {
			m_volume_info.default_window_center = 0;
			m_volume_info.default_window_width = 0;
		}

		gdcm::Tag tag_rescale_intercept(0x0028, 0x1052);
		gdcm::Tag tag_rescale_slope(0x0028, 0x1053);

		if (ds.FindDataElement(tag_rescale_intercept) && ds.FindDataElement(tag_rescale_slope)) {
			gdcm::Attribute<0x0028, 0x1052> at_rescale_intercept;
			gdcm::Attribute<0x0028, 0x1053> at_rescale_slope;
			at_rescale_intercept.Set(ds);
			at_rescale_slope.Set(ds);
			m_volume_info.rescale_intercept = at_rescale_intercept.GetValue();
			m_volume_info.rescale_slope = at_rescale_slope.GetValue();

			// For debug
			at_rescale_intercept.Print(std::cout);
			at_rescale_slope.Print(std::cout);
		}
		else {
			m_volume_info.rescale_intercept = 0;
			m_volume_info.rescale_slope = 1;
		}

		gdcm::Tag tag_image_orientation(0x0020, 0x0037);
		if (ds.FindDataElement(tag_image_orientation)) {
			gdcm::Attribute< 0x0020, 0x0037> at_image_orientation;
			at_image_orientation.Set(ds);
			const double* c = at_image_orientation.GetValues();

			m_volume_info.m_ptHor_x = c[0];
			m_volume_info.m_ptHor_y = c[1];
			m_volume_info.m_ptHor_z = c[2];
			m_volume_info.m_ptVer_x = c[3];
			m_volume_info.m_ptVer_y = c[4];
			m_volume_info.m_ptVer_z = c[5];

		}

		gdcm::Tag tag_patient_position(0x0018, 0x5100);
		if (ds.FindDataElement(tag_patient_position)) {
			gdcm::Attribute<0x0018, 0x5100> at_patient_position;
			at_patient_position.Set(ds);
			m_volume_info.patient_position = at_patient_position.GetValue() ;

		}

		// Get color representation
		/*
		gdcm::Tag tag_transfer_syntax_uid(0x0002, 0x0010);
		std::string transfer_syntax_uid = "";

		if (header.FindDataElement(tag_transfer_syntax_uid)) {
			gdcm::DataElement de = header.GetDataElement(tag_transfer_syntax_uid);
			gdcm::Attribute<0x0002, 0x0010> at_transfer_syntax_uid;
			at_transfer_syntax_uid.Set(header);

			transfer_syntax_uid = at_transfer_syntax_uid.GetValue();
		}
		*/
		//_setPhotometry(transfer_syntax_uid, m_volume_info.samples_per_pixel);
	}

	int CgipDicomIO::checkForIrregular(std::vector<std::string> files) {

		gdcm::ImageReader reader1;
		reader1.SetFileName(files[0].c_str());
		if (!reader1.Read()) {
			printf(" unable to read dicom \n");
		}


		gdcm::ImageReader reader2;
		reader2.SetFileName(files[1].c_str());

		if (!reader2.Read()) {
			printf(" unable to read dicom \n");
		}

		gdcm::ImageReader reader3;
		reader3.SetFileName(files[files.size() - 1].c_str());

		if (!reader3.Read()) {
			printf(" unable to read dicom \n");
		}


		const gdcm::File& f1 = reader1.GetFile();
		const gdcm::File& f2 = reader2.GetFile();
		const gdcm::File& f3 = reader3.GetFile();

		const gdcm::DataSet& ds1 = f1.GetDataSet();
		const gdcm::DataSet& ds2 = f2.GetDataSet();
		const gdcm::DataSet& ds3 = f3.GetDataSet();


		const double* c1;
		const double* c2;
		const double* c3;


		gdcm::Tag tag_image_orientation(0x0020, 0x0037);
		if (ds1.FindDataElement(tag_image_orientation)) {
			gdcm::Attribute< 0x0020, 0x0037> at_image_orientation;
			at_image_orientation.Set(ds1);
			c1 = at_image_orientation.GetValues();
		}
		else {
			return 0;
		}

		if (ds2.FindDataElement(tag_image_orientation)) {
			gdcm::Attribute< 0x0020, 0x0037> at_image_orientation;
			at_image_orientation.Set(ds2);
			c2 = at_image_orientation.GetValues();
		}
		else {
			return 2;
		}

		if (ds3.FindDataElement(tag_image_orientation)) {
			gdcm::Attribute< 0x0020, 0x0037> at_image_orientation;
			at_image_orientation.Set(ds3);
			c3 = at_image_orientation.GetValues();
		}
		else {
			std::cout << "no image orientation tag" << std::endl;
			return 1;
		}


		for (int i = 0; i < 6; i++) {
			if (*c1 != *c2) {
				return 0;
			}

			if (*c3 != *c2) {
				return 1;
			}
			c1++;
			c2++;
			c3++;
		}
		
		return 2;
	}



	bool CgipDicomIO::checkInverseOrder(const std::string& file1, const std::string& file2){
		bool bInverse = false;

		gdcm::ImageReader r1;
		r1.SetFileName(file1.c_str());
		if (!r1.Read()) {
			printf("unable to read dicom\n");
			return false;
		}

		gdcm::StringFilter sf1;
		sf1.SetFile(r1.GetFile());
		const gdcm::Tag image_orientation(0x0020, 0x0032);
		auto str = sf1.ToStringPair(image_orientation).second;
		str = str.substr(str.find_last_of('\\') + 1, str.size());

		float zPos1 = std::stof(str);

		gdcm::ImageReader r2;
		r2.SetFileName(file2.c_str());
		if (!r2.Read()) {
			printf("unable to read dicom\n");
			return false;
		}
		gdcm::StringFilter sf2;
		sf2.SetFile(r2.GetFile());
		auto str2 = sf2.ToStringPair(image_orientation).second;
		str2 = str2.substr(str2.find_last_of('\\') + 1, str2.size());
		float zPos2 = std::stof(str2);

		if (zPos1 - zPos2 < 0.0f)
			bInverse = true;

		return bInverse;
	}

	void CgipDicomIO::computeSpacingZ(const char* filename1, const char* filename2) {
		// Slice 1 & 2
		gdcm::ImageReader reader1, reader2;
		reader1.SetFileName(filename1);
		reader2.SetFileName(filename2);
		if (!reader1.Read() || !reader2.Read()) {
			throw std::runtime_error("ERR!");
		}

		CgipFloat spacing_z;
		gdcm::Tag tag_image_position(0x0020, 0x0032);
		//gdcm::Tag tag_slice_thickness1(0x0018, 0x0050);
		gdcm::Tag tag_spacing_between_slices(0x0018, 0x0088);
		gdcm::File &r_f1 = reader1.GetFile();
		gdcm::File &r_f2 = reader2.GetFile();
		gdcm::DataSet &r_ds1 = r_f1.GetDataSet();
		gdcm::DataSet &r_ds2 = r_f2.GetDataSet();


		if (r_ds1.FindDataElement(tag_spacing_between_slices)) {
			gdcm::Attribute<0x0018, 0x0088> spacing_between_slices;
			spacing_between_slices.Set(r_ds1);
			spacing_z = spacing_between_slices.GetValue();
		}
		else if (r_ds1.FindDataElement(tag_image_position) && r_ds2.FindDataElement(tag_image_position)) {
			gdcm::Attribute<0x0020, 0x0032> at_image_position_patient_1;
			gdcm::Attribute<0x0020, 0x0032> at_image_position_patient_2;
			at_image_position_patient_1.Set(r_ds1);
			at_image_position_patient_2.Set(r_ds2);
			spacing_z = at_image_position_patient_2.GetValue(2) - at_image_position_patient_1.GetValue(2);
		}
		else {
			//throw CgipException(tag_not_found_msg(tag_image_position.PrintAsPipeSeparatedString()));
			spacing_z = 1;
		}

		if (spacing_z < 0) spacing_z *= -1;

		if (spacing_z > 1e-2 && spacing_z < 100) {
			if(spacing_z > m_volume_info.spacing_z)
				m_volume_info.spacing_z = spacing_z;
		}
		

		// For Debug
		std::cout << "calculated spacing_z: " << spacing_z << std::endl;
	}


	/**
	 *	Name:		getTagNotFoundMsg
	 *  Created:	2020-10-16
	 *	Modified:	2020-12-21
	 *	Author:		Jusang Lee, Sanguk Park
	 *	Params:
	 *		- tag_str:	std::string
	 *			tag index of dicom header
	 *	Desc:		return the error message for failed case to find tag.
	 */
	std::string getTagNotFoundMsg(std::string tag_str) {
		return "Dicom Err: Cannot find attribute (" + tag_str.substr(0, 4) + ", " + tag_str.substr(5, 4) + ")";
	}
}
