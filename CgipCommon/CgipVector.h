#pragma once

#include <math.h>
#include <Eigen/Geometry>
#include "cgip_common.h"
#include "cgip_types.h"

namespace cgip {
	//using std::operator+;

	class CGIPCOMMON_DLL CgipVector {

	public:
		CgipVector();
		CgipVector(CgipFloat vals, ...);
		CgipVector(CgipFloat x, CgipFloat y, CgipFloat z);
		CgipVector(Eigen::Vector3f v);
		Eigen::Vector3f toEigen();
		Eigen::Vector4f toEigen(CgipFloat w);

		friend CGIPCOMMON_DLL CgipVector operator+(const CgipVector & first, const CgipVector & second);
		friend CGIPCOMMON_DLL CgipVector operator*(const CgipVector& v, const CgipFloat s);
		friend CGIPCOMMON_DLL CgipVector operator*(const CgipFloat s, const CgipVector & v);
		CgipFloat dot(CgipVector& v);

		inline CgipFloat x() const { return m_x; }
		inline CgipFloat y() const { return m_y; }
		inline CgipFloat z() const { return m_z; }
		inline CgipFloat len() { return sqrt(m_x * m_x + m_y * m_y + m_z * m_z); }

		void normalize();

	private:
		CgipInt m_size;
		CgipFloat* m_data = nullptr;
		CgipFloat m_x;
		CgipFloat m_y;
		CgipFloat m_z;
	};
}