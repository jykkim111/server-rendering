#include "CgipPlane.h"

namespace cgip {
	CgipPlane::CgipPlane() {

	}

	CgipPlane::CgipPlane(CgipInt width, CgipInt height, CgipInt channel) {
		m_width = width;
		m_height = height;
		m_channel = channel; // default = 1

		// Initialize vector
		m_plane.insert(m_plane.end(), width * height * channel, 0);

		if (m_plane.data() == NULL) {
			throw CgipSlotNullException();
		}
	}


	// Copy constructor
	CgipPlane::CgipPlane(const CgipPlane& other) {
		m_width = other.m_width;
		m_height = other.m_height;
		m_channel = other.m_channel;

		m_plane = other.m_plane;
	}

	CgipPlane& CgipPlane::operator=(const CgipPlane& other) {
		m_width = other.m_width;
		m_height = other.m_height;
		m_channel = other.m_channel;

		m_plane = other.m_plane;

		return *this;
	}

	// Move constructor
	CgipPlane::CgipPlane(CgipPlane&& other) {
		m_width = other.m_width;
		m_height = other.m_height;
		m_channel = other.m_channel;

		// Move a plane vector
		m_plane = std::move(other.m_plane);

		other.m_width = 0;
		other.m_height = 0;
		other.m_channel = 0;
	}

	// Move assign operator
	CgipPlane& CgipPlane::operator=(CgipPlane&& other) {

		if (this != &other) {
			m_width = other.m_width;
			m_height = other.m_height;
			m_channel = other.m_channel;

			// Move a plane vector
			m_plane = std::move(other.m_plane);

			other.m_width = 0;
			other.m_height = 0;
			other.m_channel = 0;
		}
		return *this;
	}

	void CgipPlane::setPixelValue(CgipInt x, CgipInt y, CgipShort val) {
		setPixelValue(x, y, 0, val);
	}

	void CgipPlane::setPixelValue(CgipInt x, CgipInt y, CgipInt c, CgipShort val) {
		m_plane[m_channel * (y * m_width + x) + c] = val;
	}

	CgipShort CgipPlane::getPixelValue(CgipInt x, CgipInt y, CgipInt c) {
		return m_plane[m_channel * (y * m_width + x) + c];
	}

	std::vector<short> CgipPlane::getPlane() {
		return m_plane;
	}
}
