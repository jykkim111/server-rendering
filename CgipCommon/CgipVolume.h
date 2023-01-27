#pragma once

#include "cgip_common.h"
#include "cgip_types.h"
#include "Memory.h"

namespace cgip {

	class CGIPCOMMON_DLL CgipVolume {
	public:

		/* Constructors */
		CgipVolume();
		CgipVolume(CgipInt width, CgipInt height, CgipInt depth);
		CgipVolume(CgipInt width, CgipInt height, CgipInt depth, short** data);
		~CgipVolume();

		// TODO: Need to test below constructors
		CgipVolume(const CgipVolume& other);
		CgipVolume& operator=(const CgipVolume& other);
		CgipVolume(CgipVolume&& other);
		CgipVolume& operator=(CgipVolume&& other);

		void setVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z, CgipShort val);
		CgipShort getVoxelValue(CgipFloat x, CgipFloat y, CgipFloat z, CgipShort default_val=0);
		CgipShort getMaxVoxelValue();
		CgipShort getMinVoxelValue();

		void setNumEl(int num_el);
		void setWidth(int width);
		void setHeight(int height);
		void setDepth(int depth);
		CgipInt getNumEl();
		CgipInt getWidth();
		CgipInt getHeight();
		CgipInt getDepth();

		void setSpacingX(float sp_x);
		void setSpacingY(float sp_y);
		void setSpacingZ(float sp_z);
		CgipFloat getSpacingX();
		CgipFloat getSpacingY();
		CgipFloat getSpacingZ();

		CgipBool isOutbound(CgipFloat x, CgipFloat y, CgipFloat z);

		inline short** getRawPtr() { return m_volume; }

	protected:
		CgipInt m_depth;
		CgipInt m_height;
		CgipInt m_width;
		CgipInt m_numel;

		CgipFloat m_spacing_x;
		CgipFloat m_spacing_y;
		CgipFloat m_spacing_z;

		CgipBool m_is_reference = false;

	private:
		CgipShort** m_volume = nullptr;

		CgipFloat interpolate_1d(CgipFloat v1, CgipFloat v2, CgipFloat x);
		CgipFloat interpolate_2d(CgipFloat v1, CgipFloat v2, CgipFloat v3, CgipFloat v4,
			CgipFloat x, CgipFloat y);
		CgipFloat interpolate_3d(CgipFloat v1, CgipFloat v2, CgipFloat v3, CgipFloat v4, CgipFloat v5, CgipFloat v6, CgipFloat v7, CgipFloat v8,
			CgipFloat x, CgipFloat y, CgipFloat z);
	};
}