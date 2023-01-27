#include "CgipImageIO.h"

#include <png.hpp>

#include <Windows.h>

namespace cgip {
	CgipImageIO::CgipImageIO(std::string ext) :CgipFileIO() {
		m_ext = ext;
	}

	// TODO: read raw vol
	void CgipImageIO::readVolume(std::vector<std::string> files) {

	}

	void CgipImageIO::readPlane(std::string file) {

	}

	void CgipImageIO::saveVolume(CgipVolume& vol) {

	}

	void CgipImageIO::savePlane(std::string file, CgipPlane& plane) {
		if (m_image_type == JPEG) {
			return _saveJPEGPlane(plane, file);
		}
		else if (m_image_type == RAW) {
			return _saveJPEGPlane(plane, file);
		}
		else {
		throw CgipException("ImageIO: Not supported ext");
		}
	}

	CgipPlane CgipImageIO::readPlane(
		std::string file,
		CgipPlane* old_plane)
	{
		if (m_image_type == PNG) {
			return _readPNG(file, old_plane);
		}
		else if (m_image_type == JPEG) {
			return _readJPEG(file, old_plane);
		}
		else if (m_image_type == RAW) {
			return _readJPEG(file, old_plane);
		}
		else {
			throw CgipException("ImageIO: Not supported ext");
		}
	}

	/**
	 *	Name:		readMask
	 *	Created:	2021-01-21
	 *	Modified:	2021-01-21
	 *  Author:		CHAEN LEE
	 *	Params:
	 *		- file:		std::string
	 *			file path for png
	 *		- old_mask			CgipPlane*
	 *			plane for png image
	 *		- vol_uid_list		std::string
	 *	Return:
	 *		- new_plane		CgipPlane
	 *	Desc:		load 2D mask
	 */
	CgipPlane CgipImageIO::readMask(
		std::string file,
		CgipPlane* old_mask,
		std::string vol_uid_list)
	{
		if (m_image_type == PNG) {
			return _readPNG(file, old_mask);
		}
		else if (m_image_type == JPEG) {
			return _readJPEG(file, old_mask);
		}
		else {
			throw CgipException("ImageIO: Not supported ext");
		}
	}

	CgipMask CgipImageIO::readMask(
		std::string file, 
		CgipMask* old_mask, 
		std::vector<std::string> vol_uid_list) 
	{
		throw CgipException("ImageIO: Not supported function!");
	}

	CgipMask CgipImageIO::readMask(
		std::vector<std::string> files,
		CgipMask* old_mask,
		std::vector<std::string> vol_uid_list)
	{
		if (m_image_type == PNG) {
			return _readPNGMask(files, old_mask);
		}
		else {
		throw CgipException("ImageIO: Not supported ext");
		}

	}

	void CgipImageIO::saveVolume(CgipMask& mask, std::vector<std::string> file_list) {
		CgipShort** data = mask.getRawArray();

		_savePNGVolume(data, mask.getDepth(), mask.getHeight(), mask.getWidth(), file_list);
	}

	CgipMask CgipImageIO::_readPNGMask(
		std::vector<std::string> files,
		CgipMask* old_mask)
	{
		int mask_width = old_mask->getWidth();
		int mask_height = old_mask->getHeight();
		int mask_depth = old_mask->getDepth();

		CgipMask* new_mask = new CgipMask(mask_width, mask_height, mask_depth);

		for (int i = 0; i < files.size(); i++) {
			auto file = files[i];

			if (file == "") continue;

			png::image<png::gray_pixel> image(file);
			if (mask_width != image.get_width() || mask_height != image.get_height())
				throw CgipException("ImageIO Err: Volume and Mask have different width & height");

			// convert to CgipMask
			auto pix_buf = image.get_pixbuf();
#pragma omp parallel for
			for (int xy = 0; xy < mask_width * mask_height; xy++) {
				int y = xy / mask_width;
				int x = xy % mask_width;
				int val = pix_buf.get_pixel(x, y);
				new_mask->setVoxelValue(x, y, i, val);
			}
		}

		return std::move(*new_mask);
	}

	/**
	 *	Name:		_readPNG
	 *	Created:	2021-01-21
	 *	Modified:	2021-01-21
	 *  Author:		CHAEN LEE
	 *	Params:
	 *		- file:		std::string
	 *			file path for png
	 *		- old_plane			CgipPlane*
	 *			plane for png image
	 *	Return:
	 *		- new_plane		CgipPlane
	 *	Desc:		load png file image
	 */
	CgipPlane CgipImageIO::_readPNG(
		std::string file,
		CgipPlane* old_plane)
	{
		int plane_width = old_plane->getWidth();
		int plane_height = old_plane->getHeight();

		CgipPlane* new_plane = new CgipPlane(plane_width, plane_height);

		png::image<png::gray_pixel> image(file);
		if (plane_width != image.get_width() || plane_height != image.get_height())
			throw CgipException("ImageIO Err: Volume and Mask have different width & height");

		// convert to CgipMask
		auto pix_buf = image.get_pixbuf();
#pragma omp parallel for
		for (int xy = 0; xy < plane_width * plane_height; xy++) {
			int y = xy / plane_width;
			int x = xy % plane_width;
			int val = pix_buf.get_pixel(x, y);
			new_plane->setPixelValue(x, y, val);
		}

		return std::move(*new_plane);
	}

	CgipPlane CgipImageIO::_readJPEG(
		std::string file,
		CgipPlane* old_plane)
	{
		int plane_width = old_plane->getWidth();
		int plane_height = old_plane->getHeight();
		int plane_channel = old_plane->getChannel();

		CgipPlane* new_plane = new CgipPlane(plane_width, plane_height, plane_channel);

		bitmap_image image(file);

		if (!image) {
			throw CgipException("ImageIO Err: failed to open");
		}

		if (plane_width != image.height() || plane_height != image.width()) {
			throw CgipException("ImageIO Err: Volume and Mask have different width & height");
		}

		for (std::size_t y = 0; y < image.height(); ++y) {
			for (std::size_t x = 0; x < image.width(); ++x) {
				rgb_t colour;

				image.get_pixel(x, y, colour);

				new_plane->setPixelValue(x, y, 0, colour.red);
				new_plane->setPixelValue(x, y, 1, colour.green);
				new_plane->setPixelValue(x, y, 2, colour.blue);
			}
		}

		return std::move(*new_plane);
	}

	void CgipImageIO::_saveJPEGPlane(CgipPlane& plane, std::string file) {
		int plane_width = plane.getWidth();
		int plane_height = plane.getHeight();

		bitmap_image image(plane_width, plane_height);
		image_drawer draw(image);

		for (int i = 0; i < image.width(); ++i)
		{
			for (int j = 0; j < image.height(); ++j)
			{
				image.set_pixel(i, j, plane.getPixelValue(i, j, 0), plane.getPixelValue(i, j, 1), plane.getPixelValue(i, j, 2));
			}
		}

		image.save_image(file);
	}

	void CgipImageIO::_savePNGVolume(CgipShort** data, int depth, int height, int width, std::vector<std::string> file_list) {
		png::image<png::gray_pixel> image(width, height);

		// TODO: using parallel
		for (int z = 0; z < depth; z++) {
			for (size_t y = 0; y < image.get_height(); y++) {
				for (size_t x = 0; x < image.get_width(); x++) {
					png::gray_pixel pix = data[z][y * width + x];
					image[y][x] = pix;
				}
			}

			std::string filename = file_list[z];

			// Save slice by slice
			image.write(filename.c_str());
		}
	}
}