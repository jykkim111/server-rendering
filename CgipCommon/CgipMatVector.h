#pragma once
#include <assert.h>
#include "cgip_common.h"
#include "cgip_types.h"
#include "../CgipException/CgipException.h"
// RUTOS
/* Structure ******************************************************************/

/* #define Directive **********************************************************/
//#define TRUE    1
//#define FALSE   0

#ifndef    M_PI
#define    M_PI    3.14159265358979323846
#endif

#define ABS(x)   ( ((x)>0.0) ? (x) :-(x) )
#define ACOS(x)  ( ((x)>1.0) ? (0) : ( ((x)<-1.0) ? (M_PI) : (acos(x)) ) )
#define ASIN(x)  ( ((x)>1.0) ? (M_PI/2.0) : ( ((x)<-1.0) ? (-M_PI/2.0) : (asin(x)) ) )
#define SQR(x)   ( (x)*(x) )
#define SHIFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

/* Reference class ************************************************************/

namespace cgip {

    class CgipMatrix;

    /* Class declaration **********************************************************/
    class CGIPCOMMON_DLL CgipMatVector {
    private:
        int m_n, m_on;
        double* m_pv;
        int* m_pnIndex;

        // stream
        //  friend ostream&  operator<<( ostream&, vectorN const& );
        //  friend istream&  operator>>( istream&, vectorN& );

        // dot product
        friend double operator%(CgipMatVector const&, CgipMatVector const&);

    public:
        CgipMatVector();
        CgipMatVector(int);
        CgipMatVector(int, double*);
        ~CgipMatVector();

        void      GetValue(double*);
        double      GetValue(int i) const { assert(i >= 0 && i < m_n); return m_pv[i]; }
        void      SetValue(double*);
        void      SetValue(int i, double d) { assert(i >= 0 && i < m_n); m_pv[i] = d; }

        double& operator[](int i) const { assert(i >= 0 && i < m_n); return m_pv[i]; }
        int       Size() const { return m_n; }
        int       GetSize() const { return m_n; }
        void      SetSize(int);

        double    Len() const;
        double    Length() const;

        CgipMatVector& Normalize();

        CgipMatVector& Assign(CgipMatVector const&);
        CgipMatVector& operator=(CgipMatVector const&);

        CgipMatVector& Negate();

        CgipMatVector& Add(CgipMatVector const&, CgipMatVector const&);
        CgipMatVector& operator+=(CgipMatVector const&);

        CgipMatVector& Sub(CgipMatVector const&, CgipMatVector const&);
        CgipMatVector& operator-=(CgipMatVector const&);

        CgipMatVector& Mult(CgipMatVector const&, double);
        CgipMatVector& operator*=(double);

        CgipMatVector& Mult(CgipMatrix const&, CgipMatVector const&);
        CgipMatVector& Mult(CgipMatVector const&, CgipMatrix const&);


        CgipMatVector& Div(CgipMatVector const&, double);
        CgipMatVector& operator/=(double);

        friend double Difference(CgipMatVector const&, CgipMatVector const&);

        // SOR (Successive Over Relaxation)
        CgipMatVector& Solve(CgipMatrix const&, CgipMatVector const&, int, double, double);

        // LU Decomposition
        CgipMatVector& Solve(CgipMatrix const&, CgipMatVector const&);

        // SVD (Singular Value Decomposition)
        CgipMatVector& Solve(CgipMatrix const&, CgipMatVector const&, double);

    };
}
