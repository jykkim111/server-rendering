

/* include files **************************************************************/
//#include "StdAfx.h"
//#include "mathclass.h"
#include <math.h>
#include <assert.h>
#include "CgipMatVector.h"
#include "CgipMatrix.h"


/* #define Preprocessor *******************************************************/


/* Constant *******************************************************************/


/* Function definition ********************************************************/

namespace cgip {
	CgipMatVector::CgipMatVector()
	{
		m_on = m_n = 0;
		m_pnIndex = nullptr;
	}

	CgipMatVector::CgipMatVector(int x)
	{
		m_on = m_n = x;
		if (m_on > 0) m_pv = new double[m_n];
	}

	CgipMatVector::CgipMatVector(int x, double* a)
	{
		m_on = m_n = x;
		m_pv = new double[m_n];
		for (int i = 0; i < m_n; i++)
			m_pv[i] = a[i];
	}

	CgipMatVector::~CgipMatVector()
	{
		if (m_on > 0) delete[] m_pv;
		if (m_pnIndex) delete[] m_pnIndex;
	}

	void CgipMatVector::SetValue(double* d)
	{
		for (int i = 0; i < m_n; i++)
			m_pv[i] = d[i];
	}

	void CgipMatVector::GetValue(double* d)
	{
		for (int i = 0; i < m_n; i++)
			d[i] = m_pv[i];
	}

	void CgipMatVector::SetSize(int x)
	{
		if (m_on < x)
		{
			if (m_on > 0) delete[] m_pv;
			m_pv = new double[x];
			m_on = x;
		}
		m_n = x;
	}

	CgipMatVector& CgipMatVector::operator=(CgipMatVector const& a)
	{
		CgipMatVector& c = (*this);
		c.SetSize(a.Size());

		for (int i = 0; i < c.Size(); i++)
			c[i] = a[i];
		return c;
	}

	CgipMatVector& CgipMatVector::Assign(CgipMatVector const& a)
	{
		CgipMatVector& c = (*this);
		c.SetSize(a.Size());

		for (int i = 0; i < c.Size(); i++)
			c[i] = a[i];
		return c;
	}

	CgipMatVector& CgipMatVector::Negate()
	{
		CgipMatVector& c = (*this);
		for (int i = 0; i < c.Size(); i++)
			c[i] = -c[i];
		return c;
	}

	CgipMatVector& CgipMatVector::Add(CgipMatVector const& a, CgipMatVector const& b)
	{
		CgipMatVector& c = (*this);
		assert(a.Size() == b.Size());
		c.SetSize(a.Size());

		for (int i = 0; i < a.Size(); i++)
			c[i] = a[i] + b[i];
		return c;
	}

	CgipMatVector& CgipMatVector::operator+=(CgipMatVector const& a)
	{
		CgipMatVector& c = (*this);
		assert(c.Size() == a.Size());

		for (int i = 0; i < c.Size(); i++)
			c[i] += a[i];
		return c;
	}

	CgipMatVector& CgipMatVector::Sub(CgipMatVector const& a, CgipMatVector const& b)
	{
		CgipMatVector& c = (*this);
		assert(a.Size() == b.Size());
		c.SetSize(a.Size());

		for (int i = 0; i < a.Size(); i++)
			c[i] = a[i] - b[i];
		return c;
	}

	CgipMatVector& CgipMatVector::operator-=(CgipMatVector const& a)
	{
		CgipMatVector& c = (*this);
		assert(c.Size() == a.Size());

		for (int i = 0; i < a.Size(); i++)
			c[i] -= a[i];
		return c;
	}

	double operator%(CgipMatVector const& a, CgipMatVector const& b)
	{
		assert(a.Size() == b.Size());

		double c = 0;
		for (int i = 0; i < a.Size(); i++)
			c += a[i] * b[i];
		return c;
	}

	CgipMatVector& CgipMatVector::Mult(CgipMatVector const& b, double a)
	{
		CgipMatVector& c = (*this);
		c.SetSize(b.Size());

		for (int i = 0; i < c.Size(); i++)
			c[i] = b[i] * a;
		return c;
	}

	CgipMatVector& CgipMatVector::operator*=(double a)
	{
		CgipMatVector& c = (*this);

		for (int i = 0; i < c.Size(); i++)
			c[i] *= a;
		return c;
	}

	CgipMatVector& CgipMatVector::Div(CgipMatVector const& b, double a)
	{
		CgipMatVector& c = (*this);
		c.SetSize(b.Size());

		for (int i = 0; i < c.Size(); i++)
			c[i] = b[i] / a;
		return c;
	}

	CgipMatVector& CgipMatVector::operator/=(double a)
	{
		CgipMatVector& c = (*this);

		for (int i = 0; i < c.Size(); i++)
			c[i] /= a;
		return c;
	}

	CgipMatVector& CgipMatVector::Mult(CgipMatrix const& a, CgipMatVector const& b)
	{
		CgipMatVector& c = (*this);
		assert(a.Column() == b.Size());
		c.SetSize(a.Row());

		for (int i = 0; i < a.Row(); i++)
		{
			c[i] = 0;
			for (int k = 0; k < b.Size(); k++)
				c[i] += a[i][k] * b[k];
		}

		return c;
	}

	CgipMatVector& CgipMatVector::Mult(CgipMatVector const& b, CgipMatrix const& a)
	{
		CgipMatVector& c = (*this);
		assert(a.Row() == b.Size());
		c.SetSize(a.Column());

		for (int i = 0; i < a.Column(); i++)
		{
			c[i] = 0;
			for (int k = 0; k < b.Size(); k++)
				c[i] += b[k] * a[k][i];
		}

		return c;
	}


	double CgipMatVector::Length() const
	{
		double c = 0;
		for (int i = 0; i < m_n; i++)
			c += this->m_pv[i] * this->m_pv[i];
		return sqrt(c);
	}

	double CgipMatVector::Len() const
	{
		return this->Length();
	}

	CgipMatVector& CgipMatVector::Normalize()
	{
		CgipMatVector& c = (*this);

		double l = this->Len();
		for (int i = 0; i < m_n; i++)
			c[i] = c[i] / l;
		return c;
	}

	double Difference(CgipMatVector const& a, CgipMatVector const& b)
	{
		assert(a.GetSize() == b.GetSize());

		double d = 0.0;
		for (int i = 0; i < a.GetSize(); i++)
			d += (a.m_pv[i] - b.m_pv[i]) * (a.m_pv[i] - b.m_pv[i]);

		return d;
	}

	CgipMatVector& CgipMatVector::Solve(CgipMatrix const& a, CgipMatVector const& b, int num,
		double tolerance, double damp)
	{
		CgipMatVector& c = (*this);
		assert(a.Row() == a.Column());
		assert(a.Row() == b.Size());
		c.SetSize(b.Size());

		int flag = 1;
		for (int i = 0; i < num && flag; i++)
		{
			flag = 0;
			for (int j = 0; j < a.Row(); j++)
			{
				double r = b[j] - a[j] % c;
				c[j] += damp * r / a[j][j];
				if (r > tolerance) flag = 1;
			}
		}

		return c;
	}


	CgipMatVector& CgipMatVector::Solve(CgipMatrix const& a, CgipMatVector const& b)
	{
		CgipMatVector& c = (*this);
		assert(a.Row() == a.Column());
		assert(a.Row() == b.Size());

		int n = b.Size();
		c.SetSize(n);
		c.Assign(b);

		static CgipMatrix mat;
		mat.SetSize(n, n);
		mat.Assign(a);

		//	static int* index;
		//	static int index_count = 0;
		int index_count = 0;
		if (index_count < n)
		{
			if (index_count > 0) delete[] m_pnIndex;
			index_count = n;
			if (index_count > 0) m_pnIndex = new int[index_count];
		}

		mat.LUdecompose(m_pnIndex);
		mat.LUsubstitute(m_pnIndex, c);

		return c;
	}


	CgipMatVector& CgipMatVector::Solve(CgipMatrix const& a, CgipMatVector const& b, double tolerance)
	{
		int m = a.Row();
		int n = a.Column();

		assert(m >= n);
		assert(b.Size() == m);

		CgipMatVector& c = (*this);
		c.SetSize(n);

		static CgipMatrix u; u.SetSize(m, n);
		static CgipMatVector w; w.SetSize(n);
		static CgipMatrix v; v.SetSize(n, n);

		u.Assign(a);
		u.SVdecompose(w, v);

		int i, j;
		double s;
		static CgipMatVector tmp;
		tmp.SetSize(n);

		double wmax = 0.0f;
		for (j = 0; j < n; j++)
			if (w[j] > wmax) wmax = w[j];

		for (j = 0; j < n; j++)
			if (w[j] < wmax * tolerance) w[j] = 0.0f;

		for (j = 0; j < n; j++)
		{
			s = 0.0f;
			if (w[j])
			{
				for (i = 0; i < m; i++)
					s += u[i][j] * b[i];
				s /= w[j];
			}
			tmp[j] = s;
		}

		for (j = 0; j < n; j++)
		{
			s = 0.0;
			for (i = 0; i < n; i++)
				s += v[j][i] * tmp[i];
			c[j] = s;
		}

		return c;
	}
}

