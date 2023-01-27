#pragma once

#include "cgip_common.h"
#include "cgip_types.h"
#include "CgipVolume.h"
#include "Memory.h"

namespace cgip {
	enum E_MaskValue
	{
		ZERO = 0,
		ONE = 1
	};

	const BYTE _acBitFlags[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	class CGIPCOMMON_DLL SegMask : public CgipVolume {
	public:
		SegMask(CgipInt width, CgipInt height, CgipInt depth);
		SegMask(CgipInt width, CgipInt height, CgipInt depth, CgipInt bufferWidth);
		SegMask(CgipInt width, CgipInt height, CgipInt depth, BYTE** data);
		~SegMask();

		// TODO: Need to test below constructors
		SegMask(const SegMask& other);
		SegMask& operator=(const SegMask& other);
		SegMask(SegMask&& other);
		SegMask& operator=(SegMask&& other);

		E_MaskValue getVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z);
		void setVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z, E_MaskValue val);

		inline BYTE** getRawPtr() { return m_mask; }
		// Copy the raw data array
		BYTE** getRawArray();

		__forceinline static E_MaskValue getValue_byBitPos(int nBitPos, BYTE cValue)
		{
			if (cValue & _acBitFlags[nBitPos])
				return ONE;

			return ZERO;
		}

		BYTE** m_mask=nullptr;
		int m_nBufferWidth;		// dimensions	
	};
}