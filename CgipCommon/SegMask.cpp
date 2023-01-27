#include <string.h>
#include <math.h>

#include "SegMask.h"
#include "../CgipException/CgipException.h"

namespace cgip {
	SegMask::SegMask(CgipInt width, CgipInt height, CgipInt depth) : CgipVolume() {

		m_width = width;
		m_height = height;
		m_depth = depth;
		m_numel = depth * height * width;

		m_nBufferWidth = width / 8;

		// Allocate array for saving voxels
		m_mask = SAFE_ALLOC_3D(BYTE, m_nBufferWidth, m_height, m_depth);
	}

	SegMask::SegMask(CgipInt width, CgipInt height, CgipInt depth, CgipInt bufferWidth) : CgipVolume() {

		m_width = width;
		m_height = height;
		m_depth = depth;
		m_numel = depth * height * width;

		m_nBufferWidth = bufferWidth;

		m_is_reference = true;

		// Allocate array for saving voxels
		m_mask = SAFE_ALLOC_3D(BYTE, m_nBufferWidth, m_height, m_depth);
	}

	SegMask::SegMask(CgipInt width, CgipInt height, CgipInt depth, BYTE** data) : CgipVolume() {

		m_depth = depth;
		m_height = height;
		m_width = width;
		m_numel = depth * height * width;

		m_is_reference = true;

		m_mask = (BYTE**)data;
	}

	SegMask::~SegMask() {
		if (!m_is_reference && m_mask != nullptr) {
			SAFE_DELETE_VOLUME(m_mask, m_depth);
		}
	}

	SegMask::SegMask(const SegMask& other) {
		m_depth = other.m_depth;
		m_height = other.m_height;
		m_width = other.m_width;
		m_nBufferWidth = other.m_nBufferWidth;

		m_numel = other.m_numel;

		m_is_reference = other.m_is_reference;

		if (m_is_reference) {
			m_mask = other.m_mask;
		}
		else {
			SAFE_DELETE_VOLUME(m_mask, m_depth);

			m_mask = SAFE_ALLOC_3D(BYTE, m_nBufferWidth, m_height, m_depth);

			for (int i = 0; i < m_depth; i++) {
				memcpy(m_mask[i], other.m_mask[i], m_height * m_nBufferWidth * sizeof(BYTE));
			}
		}
	}

	SegMask& SegMask::operator=(const SegMask& other) {
		if (this != &other) {
			m_depth = other.m_depth;
			m_height = other.m_height;
			m_nBufferWidth = other.m_nBufferWidth;
			m_width = other.m_width;

			m_numel = other.m_numel;

			m_is_reference = other.m_is_reference;

			if (m_is_reference) {
				m_mask = other.m_mask;
			}
			else {
				SAFE_DELETE_VOLUME(m_mask, m_depth);

				m_mask = SAFE_ALLOC_3D(BYTE, m_nBufferWidth, m_height, m_depth);

				for (int i = 0; i < m_depth; i++) {
					memcpy(m_mask[i], other.m_mask[i], m_height * m_nBufferWidth * sizeof(BYTE));
				}
			}
		}

		return *this;
	}

	SegMask::SegMask(SegMask&& other) {
		m_depth = other.m_depth;
		m_height = other.m_height;
		m_width = other.m_width;
		m_nBufferWidth = other.m_nBufferWidth;

		m_numel = other.m_numel;

		m_is_reference = other.m_is_reference;

		m_mask = other.m_mask;
		other.m_mask = nullptr;
	}

	SegMask& SegMask::operator=(SegMask&& other) {
		if (this != &other) {
			m_depth = other.m_depth;
			m_height = other.m_height;
			m_width = other.m_width;
			m_nBufferWidth = other.m_nBufferWidth;

			m_numel = other.m_numel;

			m_is_reference = other.m_is_reference;

			m_mask = other.m_mask;
			other.m_mask = nullptr;
		}

		return *this;
	}

	E_MaskValue SegMask::getVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z) {
		// Nearest neighbor
		CgipInt xi = (int)round(x);
		CgipInt yi = (int)round(y);
		CgipInt zi = (int)round(z);

		if (isOutbound(xi, yi, zi))
			return ZERO;

		const int nLineIndex = yi * m_nBufferWidth;
		const int nIndex0 = xi >> 3;
		const int nIndex1 = xi % 8;

		return getValue_byBitPos(nIndex1, m_mask[zi][nIndex0 + nLineIndex]);
	}

	void SegMask::setVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z, E_MaskValue val) {
		// Nearest neighbor
		CgipInt xi = (int)round(x);
		CgipInt yi = (int)round(y);
		CgipInt zi = (int)round(z);

		const int nLineIndex = yi * m_nBufferWidth;
		const int nIndex0 = xi >> 3;
		const int nIndex1 = xi % 8;

		BYTE& cValue = m_mask[zi][nIndex0 + nLineIndex];
		register BYTE flag = _acBitFlags[nIndex1];

		if (val == ZERO)
			cValue &= (~flag);
		else
			cValue |= flag;
	}

	// Copy the raw data array
	BYTE** SegMask::getRawArray() {
		BYTE** mask = SAFE_ALLOC_3D(BYTE, m_nBufferWidth, m_height, m_depth);

		for (int i = 0; i < m_depth; i++) {
			memcpy(mask[i], m_mask[i], m_height * m_width * sizeof(BYTE));
		}
		return mask;
	}
};