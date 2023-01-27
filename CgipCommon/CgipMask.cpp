#include <string.h>
#include <math.h>

#include "CgipMask.h"
#include "../CgipException/CgipException.h"

namespace cgip {
	CgipMask::CgipMask(CgipInt width, CgipInt height, CgipInt depth) : CgipVolume() {

		m_depth = depth;
		m_height = height;
		m_width = width;
		m_numel = depth * height * width;

		// Allocate array for saving voxels
		m_mask = SAFE_ALLOC_3D(CgipShort, m_width, m_height, m_depth);
	}


	CgipMask::CgipMask(CgipInt width, CgipInt height, CgipInt depth, short** data) : CgipVolume() {

		m_depth = depth;
		m_height = height;
		m_width = width;
		m_numel = depth * height * width;
		
		m_is_reference = true;

		m_mask = (CgipShort**)data;
	}

	CgipMask::~CgipMask(){
		if (!m_is_reference && m_mask != nullptr) {
			SAFE_DELETE_VOLUME(m_mask, m_depth);
		}
	}

	CgipMask::CgipMask(const CgipMask& other) {
		m_depth = other.m_depth;
		m_height = other.m_height;
		m_width = other.m_width;

		m_numel = other.m_numel;

		m_is_reference = other.m_is_reference;

		if (m_is_reference) {
			m_mask = other.m_mask;
		}
		else {
			SAFE_DELETE_VOLUME(m_mask, m_depth);

			m_mask = SAFE_ALLOC_3D(CgipShort, m_width, m_height, m_depth);

			for (int i = 0; i < m_depth; i++) {
				memcpy(m_mask[i], other.m_mask[i], m_height * m_width * sizeof(CgipShort));
			}
		}
	}

	CgipMask& CgipMask::operator=(const CgipMask& other) {
		if (this != &other) {
			m_depth = other.m_depth;
			m_height = other.m_height;
			m_width = other.m_width;

			m_numel = other.m_numel;

			m_is_reference = other.m_is_reference;

			if (m_is_reference) {
				m_mask = other.m_mask;
			}
			else {
				SAFE_DELETE_VOLUME(m_mask, m_depth);

				m_mask = SAFE_ALLOC_3D(CgipShort, m_width, m_height, m_depth);

				for (int i = 0; i < m_depth; i++) {
					memcpy(m_mask[i], other.m_mask[i], m_height * m_width * sizeof(CgipShort));
				}
			}
		}

		return *this;
	}

	CgipMask::CgipMask(CgipMask&& other) {
		m_depth = other.m_depth;
		m_height = other.m_height;
		m_width = other.m_width;

		m_numel = other.m_numel;

		m_is_reference = other.m_is_reference;

		m_mask = other.m_mask;
		other.m_mask = nullptr;
	}

	CgipMask& CgipMask::operator=(CgipMask&& other) {
		if (this != &other) {
			m_depth = other.m_depth;
			m_height = other.m_height;
			m_width = other.m_width;

			m_numel = other.m_numel;

			m_is_reference = other.m_is_reference;

			m_mask = other.m_mask;
			other.m_mask = nullptr;
		}

		return *this;
	}


	CgipShort CgipMask::getVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z) {
		// Nearest neighbor
		CgipInt xi = (int)round(x);
		CgipInt yi = (int)round(y);
		CgipInt zi = (int)round(z);

		if (isOutbound(xi, yi, zi))
			return 0;

		return m_mask[zi][yi * m_width + xi];
	}

	void CgipMask::setVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z, CgipShort val) {
		// Nearest neighbor
		CgipInt xi = (int)round(x);
		CgipInt yi = (int)round(y);
		CgipInt zi = (int)round(z);
		
		if (!isOutbound(xi, yi, zi)) {
			m_mask[zi][yi * m_width + xi] = val;

		}
	}

	// Copy the raw data array
	CgipShort** CgipMask::getRawArray() {
		CgipShort** mask = SAFE_ALLOC_3D(CgipShort, m_width, m_height, m_depth);

		for (int i = 0; i < m_depth; i++) {
			memcpy(mask[i], m_mask[i], m_height * m_width * sizeof(CgipShort));
		}
		return mask;
	}
};