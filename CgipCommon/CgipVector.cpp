#include "CgipVector.h"

namespace cgip {
	CgipVector::CgipVector(){
		m_x = 0;
		m_y = 0;
		m_z = 0;
	}

	// TODO:: Geralize this vector class
	CgipVector::CgipVector(CgipFloat vals, ...) {
	}


	CgipVector::CgipVector(CgipFloat x, CgipFloat y, CgipFloat z) {
		m_x = x;
		m_y = y;
		m_z = z;
	}

	CgipVector::CgipVector(Eigen::Vector3f v) {
		m_x = v.x();
		m_y = v.y();
		m_z = v.z();
	}

	Eigen::Vector3f CgipVector::toEigen() {
		return Eigen::Vector3f(m_x, m_y, m_z);
	}

	Eigen::Vector4f CgipVector::toEigen(CgipFloat w) {
		return Eigen::Vector4f(m_x, m_y, m_z, w);
	}

	CgipVector operator+(const CgipVector & first, const CgipVector & second) {
		CgipVector res;

		res.m_x = first.m_x + second.m_x;
		res.m_y = first.m_y + second.m_y;
		res.m_z = first.m_z + second.m_z;

		return res;
	}

	CgipVector operator*(const CgipVector & v, const CgipFloat s){
		CgipVector res;

		res.m_x = s * v.m_x;
		res.m_y = s * v.m_y;
		res.m_z = s * v.m_z;

		return res;
	}

	CgipVector operator*(const CgipFloat s, const cgip::CgipVector & v) {
		CgipVector res;

		res.m_x = s * v.m_x;
		res.m_y = s * v.m_y;
		res.m_z = s * v.m_z;

		return res;
	}

	CgipFloat CgipVector::dot(CgipVector & v) {
		CgipFloat res;

		res = this->m_x * v.x() + this->m_y * v.y() + this->m_z * v.z();

		return res;
	}

	void CgipVector::normalize() {
		CgipFloat d = len();

		if (d > 0.000001) {
			m_x /= d;
			m_y /= d;
			m_z /= d;
		}
	}
}

CgipFloat t;
cgip::CgipVector A;
cgip::CgipVector C = A * t;

//cgip::CgipVector A;
//cgip::CgipVector B;
//cgip::CgipVector C = A + B;
