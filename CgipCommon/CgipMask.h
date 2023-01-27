#pragma once

#include "cgip_common.h"
#include "cgip_types.h"
#include "CgipVolume.h"
#include "Memory.h"

namespace cgip {
	class CGIPCOMMON_DLL CgipMask : public CgipVolume {
	public:
		CgipMask(CgipInt width, CgipInt height, CgipInt depth);
		CgipMask(CgipInt width, CgipInt height, CgipInt depth, short** data);
		~CgipMask();

		// TODO: Need to test below constructors
		CgipMask(const CgipMask& other);
		CgipMask& operator=(const CgipMask& other);
		CgipMask(CgipMask&& other);
		CgipMask& operator=(CgipMask&& other);

		CgipShort getVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z);
		void setVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z, CgipShort val);

		inline CgipShort** getRawPtr() { return m_mask; }
		// Copy the raw data array
		CgipShort** getRawArray();

	private:
		CgipShort** m_mask = nullptr;
	};
}