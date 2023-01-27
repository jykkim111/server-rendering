#pragma once

#include "cgip_common.h"
#include "cgip_types.h"

#include "CgipVector.h"

namespace cgip {
	class CGIPCOMMON_DLL CgipPoint {
	public:
		CgipPoint();
		CgipPoint(CgipFloat x, CgipFloat y, CgipFloat z);
		CgipPoint(const CgipPoint& p);
		CgipPoint(Eigen::Vector3f v);
		Eigen::Vector3f toEigen();
		Eigen::Vector4f toEigen(CgipFloat w);

		CgipFloat x();
		CgipFloat y();
		CgipFloat z();

		void setXYZ(CgipFloat x, CgipFloat y, CgipFloat z);

		CgipPoint operator+(const CgipPoint& p);
		CgipPoint operator+(const CgipFloat& p);
		CgipPoint operator*(const CgipPoint& p);
		CgipPoint operator*(const CgipFloat& p);
		CgipPoint& operator=(const CgipPoint& p);
		CgipVector operator-(const CgipPoint& p);

		// Point & Vector operation
		friend CGIPCOMMON_DLL CgipPoint operator+(const CgipPoint& p, const CgipVector& v);
		friend CGIPCOMMON_DLL CgipPoint operator-(const CgipPoint& p, const CgipVector& v);
	private:
		CgipFloat m_x, m_y, m_z;
	};
}