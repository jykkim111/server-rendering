#pragma once

#include "cgip_common.h"
#include "cgip_types.h"
#include "../CgipException/CgipException.h"


namespace cgip {
    /* Class declaration **********************************************************/

    class CgipMatVector;

    class CGIPCOMMON_DLL CgipMatrix
    {
    private:
        int m_n, m_m, m_on;
        CgipMatVector* m_pv;

    public:
        // constructors
        CgipMatrix();
        CgipMatrix(int, int);
        CgipMatrix(int, int, CgipMatVector*);
        ~CgipMatrix();

        // inquiry functions
        void      GetValue(double**) const;
        double    GetValue(int, int) const;
        void      SetValue(double**);
        void      SetValue(int, int, double);

        CgipMatVector& operator[](int i) const;
        int       Row()    const { return m_n; }
        int       Column() const { return m_m; }
        void      SetSize(int, int);
        void      SetRow(int, const CgipMatVector&);
        void      SetColumn(int, const CgipMatVector&);

        CgipMatrix& Transpose(CgipMatrix const&);
        //double    Det() const;

        CgipMatrix& Assign(CgipMatrix const&);
        CgipMatrix& Mult(CgipMatrix const&, CgipMatrix const&);
        CgipMatrix& operator*=(double);
        CgipMatrix& operator/=(double);
        CgipMatrix& operator+=(CgipMatrix const&);
        CgipMatrix& operator-=(CgipMatrix const&);

        CgipMatrix& MergeUpDown(CgipMatrix const&, CgipMatrix const&);
        CgipMatrix& MergeLeftRight(CgipMatrix const&, CgipMatrix const&);
        void      SplitUpDown(CgipMatrix&, CgipMatrix&);
        void      SplitLeftRight(CgipMatrix&, CgipMatrix&);
        void      SplitUpDown(CgipMatrix&, CgipMatrix&, int);
        void      SplitLeftRight(CgipMatrix&, CgipMatrix&, int);

        void      LUdecompose(int*);
        void      LUsubstitute(int*, CgipMatVector&);
        double    LUinverse(CgipMatrix&);

        void      SVdecompose(CgipMatVector&, CgipMatrix&);
        void      SVsubstitute(CgipMatVector const&, CgipMatrix const&,
            CgipMatVector const&, CgipMatVector&);
        void      SVinverse(CgipMatrix&);

    };
}
