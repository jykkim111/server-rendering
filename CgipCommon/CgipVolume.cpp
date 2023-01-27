#include <math.h>
#include <string.h>
#include <algorithm>

#include "CgipVolume.h"
#include "../CgipException/CgipException.h"

namespace cgip {
	CgipVolume::CgipVolume() {

	}

	CgipVolume::CgipVolume(CgipInt width, CgipInt height, CgipInt depth) {
		m_depth = depth;
		m_height = height;
		m_width = width;
		m_numel = depth * height * width;

		// Allocate array for saving voxels
		m_volume = SAFE_ALLOC_3D(CgipShort, m_width, m_height, m_depth);
	}

	CgipVolume::CgipVolume(CgipInt width, CgipInt height, CgipInt depth, short** data) {

		m_depth = depth;
		m_height = height;
		m_width = width;
		m_numel = depth * height * width;

		// Only for reference
		m_is_reference = true;
		m_volume = (CgipShort**)data;
	}

	CgipVolume::~CgipVolume() {
		if (!m_is_reference && m_volume != nullptr) {
			SAFE_DELETE_VOLUME(m_volume, m_depth);
		}
	}

	CgipVolume::CgipVolume(const CgipVolume& other) {

		m_depth = other.m_depth;
		m_height = other.m_height;
		m_width = other.m_width;

		m_spacing_x = other.m_spacing_x;
		m_spacing_y = other.m_spacing_y;
		m_spacing_z = other.m_spacing_z;

		m_numel = other.m_numel;

		m_is_reference = other.m_is_reference;

		if (m_is_reference) {
			m_volume = other.m_volume;
		}
		else {
			SAFE_DELETE_VOLUME(m_volume, m_depth);

			m_volume = SAFE_ALLOC_3D(CgipShort, m_width, m_height, m_depth);

			// Copy data array to member array.
			for (int i = 0; i < m_depth; i++) {
				memcpy(m_volume[i], other.m_volume[i], m_height * m_width * sizeof(CgipShort));
			}
		}
	}

	CgipVolume& CgipVolume::operator=(const CgipVolume& other) {

		if (this != &other) {
			m_depth = other.m_depth;
			m_height = other.m_height;
			m_width = other.m_width;

			m_spacing_x = other.m_spacing_x;
			m_spacing_y = other.m_spacing_y;
			m_spacing_z = other.m_spacing_z;

			m_numel = other.m_numel;

			m_is_reference = other.m_is_reference;

			if (m_is_reference) {
				m_volume = other.m_volume;
			}
			else {
				SAFE_DELETE_VOLUME(m_volume, m_depth);

				m_volume = SAFE_ALLOC_3D(CgipShort, m_width, m_height, m_depth);

				// Copy data array to member array.
				for (int i = 0; i < m_depth; i++) {
					memcpy(m_volume[i], other.m_volume[i], m_height * m_width * sizeof(CgipShort));
				}
			}
		}

		return *this;
	}

	CgipVolume::CgipVolume(CgipVolume&& other) {
		m_depth = other.m_depth;
		m_height = other.m_height;
		m_width = other.m_width;

		m_spacing_x = other.m_spacing_x;
		m_spacing_y = other.m_spacing_y;
		m_spacing_z = other.m_spacing_z;

		m_numel = other.m_numel;

		m_is_reference = other.m_is_reference;

		m_volume = other.m_volume;

		other.m_volume = nullptr;
	}

	CgipVolume& CgipVolume::operator=(CgipVolume&& other) {
		if (this != &other) {
			m_depth = other.m_depth;
			m_height = other.m_height;
			m_width = other.m_width;

			m_spacing_x = other.m_spacing_x;
			m_spacing_y = other.m_spacing_y;
			m_spacing_z = other.m_spacing_z;

			m_numel = other.m_numel;

			m_is_reference = other.m_is_reference;

			m_volume = other.m_volume;

			other.m_volume = nullptr;
		}
		return *this;
	}

	void CgipVolume::setVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z, CgipShort val) {
		// Nearest neighbor
		CgipInt xi = (int)round(x);
		CgipInt yi = (int)round(y);
		CgipInt zi = (int)round(z);

		if (!isOutbound(xi, yi, zi)) {
			m_volume[zi][yi * m_width + xi] = val;
		}
	}

	CgipShort CgipVolume::getVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z, CgipShort default_val) {

		if (isOutbound((int)floor(x), (int)floor(y), (int)floor(z))
			|| isOutbound((int)ceil(x), (int)ceil(y), (int)ceil(z))) {
			return default_val;
		}

		CgipFloat v1 = m_volume[(int)floor(z)][(int)floor(y) * m_width + (int)floor(x)];
		CgipFloat v2 = m_volume[(int)floor(z)][(int)floor(y) * m_width + (int)ceil(x)];
		CgipFloat v3 = m_volume[(int)floor(z)][(int)ceil(y) * m_width + (int)floor(x)];
		CgipFloat v4 = m_volume[(int)floor(z)][(int)ceil(y) * m_width + (int)ceil(x)];
		CgipFloat v5 = m_volume[(int)ceil(z)][(int)floor(y) * m_width + (int)floor(x)];
		CgipFloat v6 = m_volume[(int)ceil(z)][(int)floor(y) * m_width + (int)ceil(x)];
		CgipFloat v7 = m_volume[(int)ceil(z)][(int)ceil(y) * m_width + (int)floor(x)];
		CgipFloat v8 = m_volume[(int)ceil(z)][(int)ceil(y) * m_width + (int)ceil(x)];

		return interpolate_3d(v1, v2, v3, v4, v5, v6, v7, v8, x - floor(x), y - floor(y), z - floor(z));
	}

	CgipShort CgipVolume::getMaxVoxelValue() {
		CgipShort max_val = SHRT_MIN;

#pragma omp parallel
		{
			int max_val_thread = SHRT_MIN;
#pragma omp for
			for (CgipInt z = 0; z < m_depth; z++) {
				for (int y = 0; y < m_height; y++) {
					for (int x = 0; x < m_width; x++) {
						int val = m_volume[z][y * m_width + x];
						if (val > max_val_thread) max_val_thread = val;
					}
				}
			}
#pragma omp critical
			{
				if (max_val_thread > max_val)
					max_val = max_val_thread;
			}

		}

		return max_val;
	}

	CgipShort CgipVolume::getMinVoxelValue() {
		CgipShort min_val = SHRT_MAX;

#pragma omp parallel
		{
			int min_val_thread = SHRT_MAX;
#pragma omp for
			for (CgipInt z = 0; z < m_depth; z++) {
				for (int y = 0; y < m_height; y++) {
					for (int x = 0; x < m_width; x++) {
						int val = m_volume[z][y * m_width + x];
						if (val < min_val_thread) min_val_thread = val;
					}
				}
			}
#pragma omp critical
			{
				if (min_val_thread < min_val)
					min_val = min_val_thread;
			}

		}

		return min_val;
	}

	void CgipVolume::setNumEl(int num_el) {
		m_numel = num_el;
	}
	void CgipVolume::setWidth(int width) {
		m_width = width;
	}
	void CgipVolume::setHeight(int height) {
		m_height = height;
	}
	void CgipVolume::setDepth(int depth) {
		m_depth = depth;
	}
	CgipInt CgipVolume::getNumEl() {
		return m_numel;
	}
	CgipInt CgipVolume::getWidth() {
		return m_width;
	}
	CgipInt CgipVolume::getHeight() {
		return m_height;
	}
	CgipInt CgipVolume::getDepth() {
		return m_depth;
	}

	void CgipVolume::setSpacingX(float sp_x) {
		m_spacing_x = sp_x;
	}
	void CgipVolume::setSpacingY(float sp_y) {
		m_spacing_y = sp_y;
	}
	void CgipVolume::setSpacingZ(float sp_z) {
		m_spacing_z = sp_z;
	}

	CgipFloat CgipVolume::getSpacingX() {
		return m_spacing_x;
	}

	CgipFloat CgipVolume::getSpacingY() {
		return m_spacing_y;
	}

	CgipFloat CgipVolume::getSpacingZ() {
		return m_spacing_z;
	}

	CgipBool CgipVolume::isOutbound(CgipFloat x, CgipFloat y, CgipFloat z) {
		return (x < 0 || y < 0 || z < 0 || x >= m_width || y >= m_height || z >= m_depth);
	}


	/* Private members */

	CgipFloat CgipVolume::interpolate_1d(CgipFloat v1, CgipFloat v2, CgipFloat x) {
		return v1 * (1 - x) + v2 * x;
	}

	CgipFloat CgipVolume::interpolate_2d(CgipFloat v1, CgipFloat v2, CgipFloat v3, CgipFloat v4,
		CgipFloat x, CgipFloat y) {
		CgipFloat s = interpolate_1d(v1, v2, x);
		CgipFloat t = interpolate_1d(v3, v4, x);

		return interpolate_1d(s, t, y);
	}

	CgipFloat CgipVolume::interpolate_3d(CgipFloat v1, CgipFloat v2, CgipFloat v3, CgipFloat v4, CgipFloat v5, CgipFloat v6, CgipFloat v7, CgipFloat v8,
		CgipFloat x, CgipFloat y, CgipFloat z) {
		CgipFloat s = interpolate_2d(v1, v2, v3, v4, x, y);
		CgipFloat t = interpolate_2d(v5, v6, v7, v8, x, y);

		return interpolate_1d(s, t, z);
	}

};