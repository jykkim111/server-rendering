#pragma once

#include "cgip_common.h"
#include "cgip_types.h"
#include "CgipPoint.h"
#include "../CgipException/CgipException.h"

#include <vector>

namespace cgip {
	class CGIPCOMMON_DLL CgipPlane {
	public:
		/* Constructors */
		CgipPlane();
		CgipPlane(CgipInt width, CgipInt height, CgipInt channel=1);

		// Copy Constructor
		CgipPlane(const CgipPlane& other);
		CgipPlane& operator=(const CgipPlane& other);

		// Move Constructor
		CgipPlane(CgipPlane&& other);
		CgipPlane& operator=(CgipPlane&& other);

		inline void setWidth(CgipInt width) { m_width = width; }
		inline CgipInt getWidth() { return m_width; }
		inline void setHeight(CgipInt height) { m_height = height; }
		inline CgipInt getHeight() { return m_height; }
		inline void setChannel(CgipInt channel) { m_channel = channel; }
		inline CgipInt getChannel() { return m_channel; }

		void setPixelValue(CgipInt x, CgipInt y, CgipShort val);
		void setPixelValue(CgipInt x, CgipInt y, CgipInt c, CgipShort val);
		CgipShort getPixelValue(CgipInt x, CgipInt y, CgipInt c=0);

		std::vector<CgipShort> getPlane();

	private:
		CgipInt m_width;
		CgipInt m_height;
		CgipInt m_channel;

		std::vector<CgipShort> m_plane;
	};
}
