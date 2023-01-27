#include <math.h>
#include <assert.h>
#include "CgipMatVector.h"
#include "CgipMatrix.h"

//m_real EPS = 1.0e-4;

/* #define Preprocessor *******************************************************/
#define EPS 1.0e-4

/* Constant *******************************************************************/


/* Function definition ********************************************************/
namespace cgip {
	CgipMatrix::CgipMatrix()
	{
		m_on = m_n = m_m = 0;
	}

	CgipMatrix::CgipMatrix(int x, int y)
	{
		m_on = m_n = x;
		m_m = y;
		m_pv = new CgipMatVector[m_n];

		for (int i = 0; i < m_n; i++)
			m_pv[i].SetSize(m_m);
	}

	CgipMatrix::CgipMatrix(int x, int y, CgipMatVector* a)
	{
		m_on = m_n = x;
		m_m = y;
		m_pv = a;
	}

	CgipMatrix::~CgipMatrix()
	{
		if (m_on > 0 && m_m > 0) delete[] m_pv;
	}

	void CgipMatrix::GetValue(double** d) const
	{
		for (int i = 0; i < m_n; i++)
			for (int j = 0; i < m_m; j++)
				m_pv[i][j] = d[i][j];
	}

	double CgipMatrix::GetValue(int row, int column) const
	{
		return m_pv[row][column];
	}

	void CgipMatrix::SetValue(double** d)
	{
		for (int i = 0; i < m_n; i++)
			for (int j = 0; i < m_m; j++)
				d[i][j] = m_pv[i][j];
	}

	void CgipMatrix::SetValue(int row, int column, double value)
	{
		this->m_pv[row].SetValue(column, value);
	}

	void CgipMatrix::SetRow(int x, const CgipMatVector& vec)
	{
		for (int i = 0; i < m_m; i++)
			m_pv[x][i] = vec[i];
	}

	void CgipMatrix::SetColumn(int x, const CgipMatVector& vec)
	{
		for (int i = 0; i < m_n; i++)
			m_pv[i][x] = vec[i];
	}

	void CgipMatrix::SetSize(int x, int y)
	{
		if (m_on < x)
		{
			if (m_on > 0) delete[] m_pv;
			m_pv = new CgipMatVector[x];
			m_on = x;
			for (int i = 0; i < x; i++)
				m_pv[i].SetSize(y);
		}
		else if (m_m < y)
			for (int i = 0; i < m_on; i++)
				m_pv[i].SetSize(y);
		m_n = x;
		m_m = y;
	}

	CgipMatrix& CgipMatrix::Assign(CgipMatrix const& a)
	{
		CgipMatrix& c = (*this);
		c.SetSize(a.Row(), a.Column());

		for (int i = 0; i < a.Row(); i++)
			for (int j = 0; j < a.Column(); j++)
				c[i][j] = a[i][j];

		return c;
	}
	CgipMatVector& CgipMatrix::operator[](int i) const
	{
		assert(i >= 0 && m_n > i); return m_pv[i];
	}

	CgipMatrix& CgipMatrix::operator+=(CgipMatrix const& a)
	{
		CgipMatrix& c = (*this);
		c.SetSize(a.Row(), a.Column());

		for (int i = 0; i < a.Row(); i++)
			for (int j = 0; j < a.Column(); j++)
				c[i][j] += a[i][j];

		return c;
	}

	CgipMatrix& CgipMatrix::operator-=(CgipMatrix const& a)
	{
		CgipMatrix& c = (*this);
		c.SetSize(a.Row(), a.Column());

		for (int i = 0; i < a.Row(); i++)
			for (int j = 0; j < a.Column(); j++)
				c[i][j] -= a[i][j];

		return c;
	}

	CgipMatrix& CgipMatrix::operator*=(double a)
	{
		CgipMatrix& c = (*this);

		for (int i = 0; i < c.Row(); i++)
			for (int j = 0; j < c.Column(); j++)
				c[i][j] *= a;

		return c;
	}

	CgipMatrix& CgipMatrix::operator/=(double a)
	{
		CgipMatrix& c = (*this);

		for (int i = 0; i < c.Row(); i++)
			for (int j = 0; j < c.Column(); j++)
				c[i][j] /= a;

		return c;
	}

	CgipMatrix& CgipMatrix::Mult(CgipMatrix const& a, CgipMatrix const& b)
	{
		CgipMatrix& c = (*this);
		assert(a.Column() == b.Row());
		c.SetSize(a.Row(), b.Column());

		for (int i = 0; i < a.Row(); i++)
			for (int j = 0; j < b.Column(); j++)
			{
				c[i][j] = 0;
				for (int k = 0; k < a.Column(); k++)
					c[i][j] += a[i][k] * b[k][j];
			}

		return c;
	}

	CgipMatrix& CgipMatrix::Transpose(CgipMatrix const& a)
	{
		CgipMatrix& c = (*this);
		c.SetSize(a.Column(), a.Row());

		for (int i = 0; i < a.Row(); i++)
			for (int j = 0; j < a.Column(); j++)
				c[j][i] = a[i][j];

		return c;
	}


	void CgipMatrix::LUdecompose(int* index)
	{
		assert(this->Row() == this->Column());

		int n = this->Row();
		int i, j, k, imax;
		double big, dum, sum, temp;

		static CgipMatVector vv;
		vv.SetSize(n);
		CgipMatrix& a = (*this);

		for (i = 0; i < n; i++)
		{
			big = 0.0f;
			for (j = 0; j < n; j++)
				if ((temp = fabs(a[i][j])) > big)
					big = temp;

			if (big == 0.0f)
			{
				//			cerr << "Singular matrix in routine LUdecompose" << endl;
				assert(0);
			}

			vv[i] = 1.0f / big;
		}

		for (j = 0; j < n; j++)
		{
			for (i = 0; i < j; i++)
			{
				sum = a[i][j];
				for (k = 0; k < i; k++)
					sum -= a[i][k] * a[k][j];
				a[i][j] = sum;
			}

			big = 0.0;
			for (i = j; i < n; i++)
			{
				sum = a[i][j];
				for (k = 0; k < j; k++)
					sum -= a[i][k] * a[k][j];
				a[i][j] = sum;
				if ((dum = vv[i] * fabs(sum)) >= big)
				{
					big = dum;
					imax = i;
				}
			}

			if (j != imax)
			{
				for (k = 0; k < n; k++)
				{
					dum = a[imax][k];
					a[imax][k] = a[j][k];
					a[j][k] = dum;
				}
				vv[imax] = vv[j];
			}

			index[j] = imax;
			if (a[j][j] == 0.0f) a[j][j] = EPS;

			if (j != n)
			{
				dum = 1.0f / a[j][j];
				for (i = j + 1; i < n; i++)
					a[i][j] *= dum;
			}
		}
	}

	void CgipMatrix::LUsubstitute(int* index, CgipMatVector& b)
	{
		assert(this->Row() == this->Column());

		int n = this->Row();
		CgipMatrix& a = (*this);

		int i, ii = -1, ip, j;
		double sum;

		for (i = 0; i < n; i++)
		{
			ip = index[i];
			sum = b[ip];
			b[ip] = b[i];

			if (ii > -1)
				for (j = ii; j < i; j++)
					sum -= a[i][j] * b[j];
			else
				if (sum)
					ii = i;

			b[i] = sum;
		}

		for (i = n - 1; i >= 0; i--)
		{
			sum = b[i];
			for (j = i + 1; j < n; j++)
				sum -= a[i][j] * b[j];
			b[i] = sum / a[i][i];
		}
	}

	double CgipMatrix::LUinverse(CgipMatrix& mat)
	{
		assert(this->Row() == this->Column());

		int n = this->Row();

		static int* index;
		static int index_count = 0;
		if (index_count < n)
		{
			if (index_count > 0) delete[] index;
			index_count = n;
			if (index_count > 0) index = new int[index_count];
		}

		int i, j;

		static CgipMatVector b; b.SetSize(n);
		mat.SetSize(n, n);

		LUdecompose(index);

		double det = 0;
		for (i = 0; i < n; i++)
			det += this->m_pv[i][i];

		for (j = 0; j < n; j++)
		{
			for (i = 0; i < n; i++) b[i] = 0;
			b[j] = 1.0;

			LUsubstitute(index, b);

			for (i = 0; i < n; i++)
				mat[i][j] = b[i];
		}

		return det;
	}

	CgipMatrix& CgipMatrix::MergeUpDown(CgipMatrix const& a, CgipMatrix const& b)
	{
		assert(a.Column() == b.Column());
		CgipMatrix& c = (*this);
		c.SetSize(a.Row() + b.Row(), a.Column());

		for (int j = 0; j < a.Column(); j++)
		{
			for (int i = 0; i < a.Row(); i++)
				c[i][j] = a[i][j];

			for (int i = 0; i < b.Row(); i++)
				c[i + a.Row()][j] = b[i][j];
		}

		return c;
	}

	CgipMatrix& CgipMatrix::MergeLeftRight(CgipMatrix const& a, CgipMatrix const& b)
	{
		assert(a.Row() == b.Row());
		CgipMatrix& c = (*this);
		c.SetSize(a.Row(), a.Column() + b.Column());

		for (int i = 0; i < a.Row(); i++)
		{
			for (int j = 0; j < a.Column(); j++)
				c[i][j] = a[i][j];

			for (int j = 0; j < b.Column(); j++)
				c[i][j + a.Column()] = b[i][j];
		}

		return c;
	}

	void CgipMatrix::SplitUpDown(CgipMatrix& a, CgipMatrix& b)
	{
		assert(this->Row() % 2 == 0);
		CgipMatrix& c = (*this);
		a.SetSize(c.Row() / 2, c.Column());
		b.SetSize(c.Row() / 2, c.Column());

		for (int j = 0; j < a.Column(); j++)
		{
			for (int i = 0; i < a.Row(); i++)
				a[i][j] = c[i][j];

			for (int i = 0; i < b.Row(); i++)
				b[i][j] = c[i + a.Row()][j];
		}
	}

	void CgipMatrix::SplitLeftRight(CgipMatrix& a, CgipMatrix& b)
	{
		assert(this->Column() % 2 == 0);
		CgipMatrix& c = (*this);
		a.SetSize(c.Row(), c.Column() / 2);
		b.SetSize(c.Row(), c.Column() / 2);

		for (int i = 0; i < a.Row(); i++)
		{
			for (int j = 0; j < a.Column(); j++)
				a[i][j] = b[i][j];

			for (int j = 0; j < b.Column(); j++)
				b[i][j] = c[i][j + a.Column()];
		}
	}

	void CgipMatrix::SplitUpDown(CgipMatrix& a, CgipMatrix& b, int num)
	{
		assert(this->Row() > num);
		CgipMatrix& c = (*this);
		a.SetSize(num, c.Column());
		b.SetSize(c.Row() - num, c.Column());

		for (int j = 0; j < a.Column(); j++)
		{
			for (int i = 0; i < a.Row(); i++)
				a[i][j] = c[i][j];

			for (int i = 0; i < b.Row(); i++)
				b[i][j] = c[i + a.Row()][j];
		}
	}

	void CgipMatrix::SplitLeftRight(CgipMatrix& a, CgipMatrix& b, int num)
	{
		assert(this->Column() > num);
		CgipMatrix& c = (*this);
		a.SetSize(c.Row(), num);
		b.SetSize(c.Row(), c.Column() - num);

		for (int i = 0; i < a.Row(); i++)
		{
			for (int j = 0; j < a.Column(); j++)
				a[i][j] = b[i][j];

			for (int j = 0; j < b.Column(); j++)
				b[i][j] = c[i][j + a.Column()];
		}
	}

	double pythag(double a, double b)
	{
		double pa = fabs(a);
		double pb = fabs(b);

		if (pa > pb) return pa * sqrt(1.0f + SQR(pb / pa));
		else return (pb == 0.0f ? 0.0f : pb * sqrt(1.0f + SQR(pa / pb)));
	}

	void CgipMatrix::SVdecompose(CgipMatVector& w, CgipMatrix& v)
	{
		CgipMatrix& a = (*this);
		int m = a.Row();
		int n = a.Column();

		w.SetSize(n);
		v.SetSize(n, n);

		int flag, i, its, j, jj, k, l, nm;
		double anorm, c, f, g, h, s, scale, x, y, z;

		static CgipMatVector rv1; rv1.SetSize(n);
		g = scale = anorm = 0.0;

		for (i = 0; i < n; i++)
		{
			l = i + 1;
			rv1[i] = scale * g;
			g = s = scale = 0.0;

			if (i < m)
			{
				for (k = i; k < m; k++)
					scale += fabs(a[k][i]);

				if (scale)
				{
					for (k = i; k < m; k++)
					{
						a[k][i] /= scale;
						s += a[k][i] * a[k][i];
					}

					f = a[i][i];
					g = -SIGN(sqrt(s), f);
					h = f * g - s;
					a[i][i] = f - g;

					for (j = l; j < n; j++)
					{
						for (s = 0.0, k = i; k < m; k++)
							s += a[k][i] * a[k][j];
						f = s / h;

						for (k = i; k < m; k++)
							a[k][j] += f * a[k][i];
					}

					for (k = i; k < m; k++)
						a[k][i] *= scale;
				}
			}

			w[i] = scale * g;
			g = s = scale = 0.0;

			if (i < m && i != n - 1)
			{
				for (k = l; k < n; k++)
					scale += fabs(a[i][k]);

				if (scale)
				{
					for (k = l; k < n; k++)
					{
						a[i][k] /= scale;
						s += a[i][k] * a[i][k];
					}

					f = a[i][l];
					g = -SIGN(sqrt(s), f);
					h = f * g - s;
					a[i][l] = f - g;

					for (k = l; k < n; k++)
						rv1[k] = a[i][k] / h;

					for (j = l; j < m; j++)
					{
						for (s = 0.0, k = l; k < n; k++)
							s += a[j][k] * a[i][k];

						for (k = l; k < n; k++)
							a[j][k] += s * rv1[k];
					}

					for (k = l; k < n; k++)
						a[i][k] *= scale;
				}
			}

			float ttmp = fabs((float)w[i]) + fabs((float)rv1[i]);

			if (ttmp > anorm)
			{
				anorm = ttmp;
			}
		}

		for (i = n - 1; i >= 0; i--)
		{
			if (i < n - 1)
			{
				if (g)
				{
					for (j = l; j < n; j++)
						v[j][i] = (a[i][j] / a[i][l]) / g;

					for (j = l; j < n; j++)
					{
						for (s = 0.0, k = l; k < n; k++)
							s += a[i][k] * v[k][j];

						for (k = l; k < n; k++)
							v[k][j] += s * v[k][i];
					}
				}

				for (j = l; j < n; j++)
					v[i][j] = v[j][i] = 0.0;
			}

			v[i][i] = 1.0;
			g = rv1[i];
			l = i;
		}

		//for( i=MIN(m, n)-1; i>=0; i-- )
		int iitmp = (m > n) ? m : n;
		for (i = iitmp; i >= 0; i--)
		{
			l = i + 1;
			g = w[i];
			for (j = l; j < n; j++)
				a[i][j] = 0.0;

			if (g)
			{
				g = 1.0 / g;
				for (j = l; j < n; j++)
				{
					for (s = 0.0, k = l; k < m; k++)
						s += a[k][i] * a[k][j];

					f = (s / a[i][i]) * g;

					for (k = i; k < m; k++)
						a[k][j] += f * a[k][i];
				}

				for (j = i; j < m; j++)
					a[j][i] *= g;
			}
			else
				for (j = i; j < m; j++)
					a[j][i] = 0.0;

			++a[i][i];
		}

		for (k = n - 1; k >= 0; k--)
		{
			for (its = 1; its < 30; its++)
			{
				flag = 1;
				for (l = k; l >= 0; l--)
				{
					nm = l - 1;
					if ((double)(fabs(rv1[l]) + anorm) == anorm)
					{
						flag = 0;
						break;
					}

					if ((double)(fabs(w[nm]) + anorm) == anorm) break;
				}

				if (flag)
				{
					c = 0.0;
					s = 1.0;

					for (i = l; i <= k; i++)
					{
						f = s * rv1[i];
						rv1[i] = c * rv1[i];

						if ((double)(fabs(f) + anorm) == anorm) break;

						g = w[i];
						h = pythag(f, g);
						w[i] = h;
						h = 1.0f / h;
						c = g * h;
						s = -f * h;

						for (j = 0; j < m; j++)
						{
							y = a[j][nm];
							z = a[j][i];
							a[j][nm] = y * c + z * s;
							a[j][i] = z * c - y * s;
						}
					}
				}

				z = w[k];
				if (l == k)
				{
					if (z < 0.0)
					{
						w[k] = -z;
						for (j = 0; j < n; j++)
							v[j][k] = -v[j][k];
					}
					break;
				}

				if (its == 29)
					//				cerr << "no convergence in 30 svdcmp iterations" << endl;

					x = w[l];
				nm = k - 1;
				y = w[nm];
				g = rv1[nm];
				h = rv1[k];
				f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0f * h * y);
				g = pythag(f, 1.0f);
				f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
				c = s = 1.0f;

				for (j = l; j <= nm; j++)
				{
					i = j + 1;
					g = rv1[i];
					y = w[i];
					h = s * g;
					g = c * g;
					z = pythag(f, h);
					rv1[j] = z;
					c = f / z;
					s = h / z;
					f = x * c + g * s;
					g = g * c - x * s;
					h = y * s;
					y *= c;

					for (jj = 0; jj < n; jj++)
					{
						x = v[jj][j];
						z = v[jj][i];
						v[jj][j] = x * c + z * s;
						v[jj][i] = z * c - x * s;
					}

					z = pythag(f, h);
					w[j] = z;

					if (z)
					{
						z = 1.0f / z;
						c = f * z;
						s = h * z;
					}

					f = c * g + s * y;
					x = c * y - s * g;

					for (jj = 0; jj < m; jj++)
					{
						y = a[jj][j];
						z = a[jj][i];
						a[jj][j] = y * c + z * s;
						a[jj][i] = z * c - y * s;
					}
				}

				rv1[l] = 0.0;
				rv1[k] = f;
				w[k] = x;
			}
		}
	}

	void CgipMatrix::SVsubstitute(CgipMatVector const& w, CgipMatrix const& v,
		const CgipMatVector& b, CgipMatVector& x)
	{
		assert(this->Column() == w.GetSize());
		assert(this->Column() == v.Column());
		assert(this->Column() == v.Row());
		assert(this->Row() == b.GetSize());
		assert(this->Column() == x.GetSize());

		int m = this->Row();
		int n = this->Column();

		int jj, j, i;
		double s;
		static CgipMatVector tmp; tmp.SetSize(n);
		CgipMatrix& u = *this;

		for (j = 0; j < n; j++) {
			s = 0.0;
			if (w[j] > EPS) {
				for (i = 0; i < m; i++) s += u[i][j] * b[i];
				s /= w[j];
			}
			tmp[j] = s;
		}
		for (j = 0; j < n; j++) {
			s = 0.0;
			for (jj = 0; jj < n; jj++) s += v[j][jj] * tmp[jj];
			x[j] = s;
		}
	}

	void CgipMatrix::SVinverse(CgipMatrix& mat)
	{
		int m = this->Row();
		int n = this->Column();

		static CgipMatrix V; V.SetSize(n, n);
		static CgipMatVector w; w.SetSize(n);

		static CgipMatVector b; b.SetSize(m);
		static CgipMatVector x; x.SetSize(n);

		mat.SetSize(n, m);

		SVdecompose(w, V);
		for (int j = 0; j < m; j++)
		{
			for (int i = 0; i < m; i++) b[i] = 0;
			b[j] = 1.0;

			SVsubstitute(w, V, b, x);

			for (int i = 0; i < n; i++)
				mat[i][j] = x[i];
		}
	}
}

