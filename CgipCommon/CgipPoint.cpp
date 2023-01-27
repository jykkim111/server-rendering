#include "CgipPoint.h"

namespace cgip{
	CgipPoint::CgipPoint() {
	}

	CgipPoint::CgipPoint(CgipFloat x, CgipFloat y, CgipFloat z) {
		m_x = x;
		m_y = y;
		m_z = z;
	}

	CgipPoint::CgipPoint(const CgipPoint& p) {
		m_x = p.m_x;
		m_y = p.m_y;
		m_z = p.m_z;
	}

	CgipPoint::CgipPoint(Eigen::Vector3f v) {
		m_x = v.x();
		m_y = v.y();
		m_z = v.z();
	}

	Eigen::Vector3f CgipPoint::toEigen() {
		return Eigen::Vector3f(m_x, m_y, m_z);
	}

	Eigen::Vector4f CgipPoint::toEigen(CgipFloat w) {
		return Eigen::Vector4f(m_x, m_y, m_z, w);
	}

	CgipFloat CgipPoint::x() {
		return m_x;
	}

	CgipFloat CgipPoint::y() {
		return m_y;
	}

	CgipFloat CgipPoint::z() {
		return m_z;
	}

	void CgipPoint::setXYZ(CgipFloat x, CgipFloat y, CgipFloat z) {
		m_x = x;
		m_y = y;
		m_z = z;
	}

	CgipPoint CgipPoint::operator+(const CgipPoint& p) {
		return CgipPoint(this->m_x + p.m_x, this->m_y + p.m_y, this->m_z + p.m_z);
	}

	CgipPoint CgipPoint::operator+(const CgipFloat& p) {
		return CgipPoint(this->m_x + p, this->m_y + p, this->m_z + p);
	}

	CgipPoint CgipPoint::operator*(const CgipPoint& p) {
		return CgipPoint(this->m_x * p.m_x, this->m_y * p.m_y, this->m_z * p.m_z);
	}

	CgipPoint CgipPoint::operator*(const CgipFloat& p) {
		return CgipPoint(this->m_x * p, this->m_y * p, this->m_z * p);
	}

	CgipPoint& CgipPoint::operator=(const CgipPoint& p) {
		this->m_x = p.m_x;
		this->m_y = p.m_y;
		this->m_z = p.m_z;

		return *this;
	}

	CgipVector CgipPoint::operator-(const CgipPoint& p) {
		return CgipVector(this->m_x - p.m_x, this->m_y - p.m_y, this->m_z - p.m_z);
	}

	CgipPoint operator+(const CgipPoint& p, const CgipVector& v) {
		CgipPoint res(p.m_x + v.x(), p.m_y + v.y(), p.m_z + v.z());
		return res;
	}

	CgipPoint operator-(const CgipPoint& p, const CgipVector& v) {
		CgipPoint res(p.m_x - v.x(), p.m_y - v.y(), p.m_z - v.z());
		return res;
	}

}