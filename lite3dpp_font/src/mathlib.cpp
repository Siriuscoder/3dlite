#include <iostream>
#include <cmath>
#include "mathlib.h"

#define EPS 0.000001

namespace nw
{
    // Mat4
    Mat4::Mat4()
    {
        init(
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
            );
    }

    Mat4::Mat4(
        FPType _m00, FPType _m01, FPType _m02, FPType _m03,
        FPType _m10, FPType _m11, FPType _m12, FPType _m13,
        FPType _m20, FPType _m21, FPType _m22, FPType _m23,
        FPType _m30, FPType _m31, FPType _m32, FPType _m33
        )
    {
        init(
            _m00, _m01, _m02, _m03,
            _m10, _m11, _m12, _m13,
            _m20, _m21, _m22, _m23,
            _m30, _m31, _m32, _m33
            );
    }

    void Mat4::init(
        FPType _m00, FPType _m01, FPType _m02, FPType _m03,
        FPType _m10, FPType _m11, FPType _m12, FPType _m13,
        FPType _m20, FPType _m21, FPType _m22, FPType _m23,
        FPType _m30, FPType _m31, FPType _m32, FPType _m33
        )
    {
        m[0][0] = _m00;
        m[0][1] = _m01;
        m[0][2] = _m02;
        m[0][3] = _m03;
        m[1][0] = _m10;
        m[1][1] = _m11;
        m[1][2] = _m12;
        m[1][3] = _m13;
        m[2][0] = _m20;
        m[2][1] = _m21;
        m[2][2] = _m22;
        m[2][3] = _m23;
        m[3][0] = _m30;
        m[3][1] = _m31;
        m[3][2] = _m32;
        m[3][3] = _m33;
    }

    Mat4 Mat4::Perspective(FPType _fovy, FPType _aspect, FPType _znear, FPType _zfar)
    {
        FPType halfa = _fovy * static_cast<float>(M_PI) / 360;
        FPType F = 1 / tan(halfa);
        FPType E = F / _aspect;
        FPType A = (_znear + _zfar) / (_znear - _zfar);
        FPType B = (2 * _zfar * _znear) / (_znear - _zfar);

        return Mat4(
            E, 0, 0,  0,
            0, F, 0,  0,
            0, 0, A, -1,
            0, 0, B,  0
            );
    }

    Mat4 Mat4::Orthographic(FPType _l, FPType _r, FPType _b, FPType _t, FPType _n, FPType _f)
    {
        FPType A = 2 / (_r - _l);
        FPType B = 2 / (_t - _b);
        FPType C = -2 / (_f - _n);
        FPType D = - (_r + _l) / (_r - _l);
        FPType E = - (_t + _b) / (_t - _b);
        FPType F = - (_f + _n) / (_f - _n);

        return Mat4(
            A, 0, 0, 0,
            0, B, 0, 0,
            0, 0, C, 0,
            D, E, F, 1
            );
    }

    Mat4 Mat4::operator*(const Mat4& _r) const
    {
        return Mat4(
            m[0][0]*_r.m[0][0] + m[0][1]*_r.m[1][0] + m[0][2]*_r.m[2][0] + m[0][3]*_r.m[3][0],
            m[0][0]*_r.m[0][1] + m[0][1]*_r.m[1][1] + m[0][2]*_r.m[2][1] + m[0][3]*_r.m[3][1],
            m[0][0]*_r.m[0][2] + m[0][1]*_r.m[1][2] + m[0][2]*_r.m[2][2] + m[0][3]*_r.m[3][2],
            m[0][0]*_r.m[0][3] + m[0][1]*_r.m[1][3] + m[0][2]*_r.m[2][3] + m[0][3]*_r.m[3][3],
            m[1][0]*_r.m[0][0] + m[1][1]*_r.m[1][0] + m[1][2]*_r.m[2][0] + m[1][3]*_r.m[3][0],
            m[1][0]*_r.m[0][1] + m[1][1]*_r.m[1][1] + m[1][2]*_r.m[2][1] + m[1][3]*_r.m[3][1],
            m[1][0]*_r.m[0][2] + m[1][1]*_r.m[1][2] + m[1][2]*_r.m[2][2] + m[1][3]*_r.m[3][2],
            m[1][0]*_r.m[0][3] + m[1][1]*_r.m[1][3] + m[1][2]*_r.m[2][3] + m[1][3]*_r.m[3][3],
            m[2][0]*_r.m[0][0] + m[2][1]*_r.m[1][0] + m[2][2]*_r.m[2][0] + m[2][3]*_r.m[3][0],
            m[2][0]*_r.m[0][1] + m[2][1]*_r.m[1][1] + m[2][2]*_r.m[2][1] + m[2][3]*_r.m[3][1],
            m[2][0]*_r.m[0][2] + m[2][1]*_r.m[1][2] + m[2][2]*_r.m[2][2] + m[2][3]*_r.m[3][2],
            m[2][0]*_r.m[0][3] + m[2][1]*_r.m[1][3] + m[2][2]*_r.m[2][3] + m[2][3]*_r.m[3][3],
            m[3][0]*_r.m[0][0] + m[3][1]*_r.m[1][0] + m[3][2]*_r.m[2][0] + m[3][3]*_r.m[3][0],
            m[3][0]*_r.m[0][1] + m[3][1]*_r.m[1][1] + m[3][2]*_r.m[2][1] + m[3][3]*_r.m[3][1],
            m[3][0]*_r.m[0][2] + m[3][1]*_r.m[1][2] + m[3][2]*_r.m[2][2] + m[3][3]*_r.m[3][2],
            m[3][0]*_r.m[0][3] + m[3][1]*_r.m[1][3] + m[3][2]*_r.m[2][3] + m[3][3]*_r.m[3][3]
        );
    }

    Mat4& Mat4::operator*=(const Mat4& _r)
    {
        init(
            m[0][0]*_r.m[0][0] + m[0][1]*_r.m[1][0] + m[0][2]*_r.m[2][0] + m[0][3]*_r.m[3][0],
            m[0][0]*_r.m[0][1] + m[0][1]*_r.m[1][1] + m[0][2]*_r.m[2][1] + m[0][3]*_r.m[3][1],
            m[0][0]*_r.m[0][2] + m[0][1]*_r.m[1][2] + m[0][2]*_r.m[2][2] + m[0][3]*_r.m[3][2],
            m[0][0]*_r.m[0][3] + m[0][1]*_r.m[1][3] + m[0][2]*_r.m[2][3] + m[0][3]*_r.m[3][3],
            m[1][0]*_r.m[0][0] + m[1][1]*_r.m[1][0] + m[1][2]*_r.m[2][0] + m[1][3]*_r.m[3][0],
            m[1][0]*_r.m[0][1] + m[1][1]*_r.m[1][1] + m[1][2]*_r.m[2][1] + m[1][3]*_r.m[3][1],
            m[1][0]*_r.m[0][2] + m[1][1]*_r.m[1][2] + m[1][2]*_r.m[2][2] + m[1][3]*_r.m[3][2],
            m[1][0]*_r.m[0][3] + m[1][1]*_r.m[1][3] + m[1][2]*_r.m[2][3] + m[1][3]*_r.m[3][3],
            m[2][0]*_r.m[0][0] + m[2][1]*_r.m[1][0] + m[2][2]*_r.m[2][0] + m[2][3]*_r.m[3][0],
            m[2][0]*_r.m[0][1] + m[2][1]*_r.m[1][1] + m[2][2]*_r.m[2][1] + m[2][3]*_r.m[3][1],
            m[2][0]*_r.m[0][2] + m[2][1]*_r.m[1][2] + m[2][2]*_r.m[2][2] + m[2][3]*_r.m[3][2],
            m[2][0]*_r.m[0][3] + m[2][1]*_r.m[1][3] + m[2][2]*_r.m[2][3] + m[2][3]*_r.m[3][3],
            m[3][0]*_r.m[0][0] + m[3][1]*_r.m[1][0] + m[3][2]*_r.m[2][0] + m[3][3]*_r.m[3][0],
            m[3][0]*_r.m[0][1] + m[3][1]*_r.m[1][1] + m[3][2]*_r.m[2][1] + m[3][3]*_r.m[3][1],
            m[3][0]*_r.m[0][2] + m[3][1]*_r.m[1][2] + m[3][2]*_r.m[2][2] + m[3][3]*_r.m[3][2],
            m[3][0]*_r.m[0][3] + m[3][1]*_r.m[1][3] + m[3][2]*_r.m[2][3] + m[3][3]*_r.m[3][3]
            );
        return *this;
    }

    // Vec4
    Vec4::Vec4() :
        x(0), y(0), z(0), w(0)
    {}

    Vec4::Vec4(FPType _x, FPType _y, FPType _z, FPType _w) :
        x(_x), y(_y), z(_z), w(_w)
    {}

    FPType Vec4::norm() const
    {
        return sqrt(len());
    }

    FPType Vec4::len() const
    {
        return x*x + y*y + z*z + w*w;
    }

    Vec4 Vec4::unit() const
    {
        FPType n = norm();
        return Vec4(x / n, y / n, z / n, w / n);
    }

    Vec4 Vec4::operator+(const Vec4& _r) const
    {
        return Vec4(x + _r.x, y + _r.y, z + _r.z, w + _r.w);
    }

    Vec4& Vec4::operator+=(const Vec4& _r)
    {
        x += _r.x;
        y += _r.y;
        z += _r.z;
        w += _r.w;
        return *this;
    }

    Vec4 Vec4::operator*(const Mat4& _m) const
    {
        return Vec4(
            x*_m.m[0][0] + y*_m.m[1][0] + z*_m.m[2][0] + w*_m.m[3][0],
            x*_m.m[0][1] + y*_m.m[1][1] + z*_m.m[2][1] + w*_m.m[3][1],
            x*_m.m[0][2] + y*_m.m[1][2] + z*_m.m[2][2] + w*_m.m[3][2],
            x*_m.m[0][3] + y*_m.m[1][3] + z*_m.m[2][3] + w*_m.m[3][3]
            );
    }

    Vec4& Vec4::operator*=(const Mat4& _m)
    {
        x = x*_m.m[0][0] + y*_m.m[1][0] + z*_m.m[2][0] + w*_m.m[3][0];
        y = x*_m.m[0][1] + y*_m.m[1][1] + z*_m.m[2][1] + w*_m.m[3][1];
        z = x*_m.m[0][2] + y*_m.m[1][2] + z*_m.m[2][2] + w*_m.m[3][2];
        w = x*_m.m[0][3] + y*_m.m[1][3] + z*_m.m[2][3] + w*_m.m[3][3];
        return *this;
    }

    // Quat
    Quat::Quat() :
        w(0), x(0), y(0), z(0)
    {}

    Quat::Quat(FPType _w, FPType _x, FPType _y, FPType _z) :
        w(_w), x(_x), y(_y), z(_z)
    {}

    FPType Quat::norm() const
    {
        return sqrt(len());
    }

    FPType Quat::len() const
    {
        return w*w + x*x + y*y + z*z;
    }

    Quat Quat::unit() const
    {
        FPType n = norm();
        return Quat(w / n, x / n, y / n, z / n);
    }

    Quat Quat::conjugate() const
    {
        return Quat(w, -x, -y, -z);
    }

    Quat Quat::inverse() const
    {
        Quat ret = conjugate();
        ret /= ret.len();
        return ret;
    }

    Mat4 Quat::matrix() const
    {
        FPType xx2 = x * x * 2;
        FPType yy2 = y * y * 2;
        FPType zz2 = z * z * 2;
        FPType xy2 = x * y * 2;
        FPType xz2 = x * z * 2;
        FPType yz2 = y * z * 2;
        FPType wx2 = w * x * 2;
        FPType wy2 = w * y * 2;
        FPType wz2 = w * z * 2;

        return Mat4(
            1 - yy2 - zz2,      xy2 + wz2,      xz2 - wy2, 0,
                xy2 - wz2,  1 - xx2 - zz2,      yz2 + wx2, 0,
                xz2 + wy2,      yz2 - wx2,  1 - xx2 - yy2, 0,
                        0,              0,              0, 1
                );
    }

    Quat Quat::FromAngleAxis(FPType _angle, FPType _x, FPType _y, FPType _z)
    {
        FPType halfa = _angle * static_cast<float>(M_PI) / 360;
        FPType sinha = sin(halfa);
        FPType cosha = cos(halfa);
        return Quat(
            cosha,
            _x * sinha,
            _y * sinha,
            _z * sinha
            );
    }

    Quat Quat::operator*(const Quat& _r) const
    {
        return Quat(
            w*_r.w - x*_r.x - y*_r.y - z*_r.z,
            w*_r.x + x*_r.w + y*_r.z - z*_r.y,
            w*_r.y + y*_r.w + z*_r.x - x*_r.z,
            w*_r.z + z*_r.w + x*_r.y - y*_r.x
            );
    }

    Quat& Quat::operator*=(const Quat& _r)
    {
        FPType tw = w*_r.w - x*_r.x - y*_r.y - z*_r.z;
        FPType tx = w*_r.x + x*_r.w + y*_r.z - z*_r.y;
        FPType ty = w*_r.y + y*_r.w + z*_r.x - x*_r.z;
        FPType tz = w*_r.z + z*_r.w + x*_r.y - y*_r.x;
        w = tw;
        x = tx;
        y = ty;
        z = tz;
        return *this;
    }

    Quat Quat::operator/(FPType _s) const
    {
        return Quat(w / _s, x / _s, y / _s, z / _s);
    }

    Quat& Quat::operator/=(FPType _s)
    {
        w /= _s;
        x /= _s;
        y /= _s;
        z /= _s;
        return *this;
    }

    bool Quat::operator==(const Quat& _r) const
    {
        return std::abs(w - _r.w) < EPS
            && std::abs(x - _r.x) < EPS
            && std::abs(y - _r.y) < EPS
            && std::abs(z - _r.z) < EPS;
    }
}

