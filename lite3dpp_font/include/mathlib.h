#ifndef NWE_MATHLIB_H
#define NWE_MATHLIB_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <memory.h>
#include <assert.h>

namespace nw
{
    typedef float FPType;

    union Mat4
    {
        FPType v[16];
        FPType m[4][4];

        Mat4();

        Mat4(
            FPType _m00, FPType _m01, FPType _m02, FPType _m03,
            FPType _m10, FPType _m11, FPType _m12, FPType _m13,
            FPType _m20, FPType _m21, FPType _m22, FPType _m23,
            FPType _m30, FPType _m31, FPType _m32, FPType _m33
            );

        void init(
            FPType _m00, FPType _m01, FPType _m02, FPType _m03,
            FPType _m10, FPType _m11, FPType _m12, FPType _m13,
            FPType _m20, FPType _m21, FPType _m22, FPType _m23,
            FPType _m30, FPType _m31, FPType _m32, FPType _m33
            );

        static Mat4 Perspective(FPType _fovy, FPType _aspect, FPType _znear, FPType _zfar);
        static Mat4 Orthographic(FPType _l, FPType _r, FPType _b, FPType _t, FPType _n, FPType _f);

        Mat4 operator*(const Mat4& _r) const;
        Mat4& operator*=(const Mat4& _r);
    };

    struct Vec4
    {
        FPType x, y, z, w;

        Vec4();
        Vec4(FPType _x, FPType _y, FPType _z, FPType _w);

        FPType norm() const;
        FPType len() const;
        Vec4 unit() const;

        Vec4 operator+(const Vec4& _r) const;
        Vec4& operator+=(const Vec4& _r);
        Vec4 operator*(const Mat4& _m) const;
        Vec4& operator*=(const Mat4& _m);
    };

    struct Quat
    {
        FPType w, x, y, z;

        Quat();
        Quat(FPType _w, FPType _x, FPType _y, FPType _z);

        FPType norm() const;
        FPType len() const;
        Quat unit() const;
        Quat conjugate() const;
        Quat inverse() const;
        Mat4 matrix() const;

        static Quat FromAngleAxis(FPType _angle, FPType _x, FPType _y, FPType _z);

        Quat operator*(const Quat& _r) const;
        Quat& operator*=(const Quat& _r);
        Quat operator/(FPType _s) const;
        Quat& operator/=(FPType _s);
        bool operator==(const Quat& _r) const;
    };
}

#endif

