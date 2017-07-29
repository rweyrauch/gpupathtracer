/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_VECTOR3_H
#define PATHTRACER_VECTOR3_H

#include <cmath>
#include "ptCudaCommon.h"
#include "ptMath.h"
#include "ptStream.h"

template <typename T>
class Vector3 {
public:
    COMMON_FUNC Vector3() { }

    COMMON_FUNC Vector3(T e0, T e1, T e2)
    {
        e[0] = e0; e[1] = e1; e[2] = e2;
    }

    COMMON_FUNC inline T x() const { return e[0]; }
    COMMON_FUNC inline T& x() { return e[0]; }

    COMMON_FUNC inline T y() const { return e[1]; }
    COMMON_FUNC inline T& y() { return e[1]; }

    COMMON_FUNC inline T z() const { return e[2]; }
    COMMON_FUNC inline T& z() { return e[2]; }

    COMMON_FUNC inline T r() const { return e[0]; }
    COMMON_FUNC inline T g() const { return e[1]; }
    COMMON_FUNC inline T b() const { return e[2]; }

    COMMON_FUNC inline const Vector3& operator+() const { return *this; }

    COMMON_FUNC inline Vector3 operator-() const { return Vector3(-e[0], -e[1], -e[2]); }

    COMMON_FUNC inline T operator[](int i) const { return e[i]; }

    COMMON_FUNC inline T& operator[](int i) { return e[i]; }

    COMMON_FUNC inline Vector3& operator+=(const Vector3& v2)
    {
        e[0] += v2.e[0];
        e[1] += v2.e[1];
        e[2] += v2.e[2];
        return *this;
    }

    COMMON_FUNC inline Vector3& operator-=(const Vector3& v2)
    {
        e[0] -= v2.e[0];
        e[1] -= v2.e[1];
        e[2] -= v2.e[2];
        return *this;
    }

    COMMON_FUNC inline Vector3& operator*=(const Vector3& v2)
    {
        e[0] *= v2.e[0];
        e[1] *= v2.e[1];
        e[2] *= v2.e[2];
        return *this;
    }

    COMMON_FUNC inline Vector3& operator/=(const Vector3& v2)
    {
        e[0] /= v2.e[0];
        e[1] /= v2.e[1];
        e[2] /= v2.e[2];
        return *this;
    }

    COMMON_FUNC inline Vector3& operator*=(const T s)
    {
        e[0] *= s;
        e[1] *= s;
        e[2] *= s;
        return *this;
    }

    COMMON_FUNC inline Vector3& operator/=(const T s)
    {
        const T invS = 1 / s;
        e[0] *= invS;
        e[1] *= invS;
        e[2] *= invS;
        return *this;
    }

    COMMON_FUNC inline T length() const
    {
        return Sqrt(squared_length());
    }

    COMMON_FUNC inline T squared_length() const
    {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }

    COMMON_FUNC inline void make_unit_vector()
    {
        T k = 1 / length();
        e[0] *= k;
        e[1] *= k;
        e[2] *= k;
    }

    COMMON_FUNC virtual bool serialize(Stream* pStream) const
    {
        if (pStream == nullptr)
            return false;

        return pStream->write(e, sizeof(e));
    }

    T e[3];
};

template <typename T>
COMMON_FUNC inline Vector3<T> operator+(const Vector3<T>& v1, const Vector3<T>& v2)
{
    return Vector3<T>(v1.e[0]+v2.e[0], v1.e[1]+v2.e[1], v1.e[2]+v2.e[2]);
}

template <typename T>
COMMON_FUNC inline Vector3<T> operator-(const Vector3<T>& v1, const Vector3<T>& v2)
{
    return Vector3<T>(v1.e[0]-v2.e[0], v1.e[1]-v2.e[1], v1.e[2]-v2.e[2]);
}

template <typename T>
COMMON_FUNC inline Vector3<T> operator*(const Vector3<T>& v1, const Vector3<T>& v2)
{
    return Vector3<T>(v1.e[0]*v2.e[0], v1.e[1]*v2.e[1], v1.e[2]*v2.e[2]);
}

template <typename T>
COMMON_FUNC inline Vector3<T> operator/(const Vector3<T>& v1, const Vector3<T>& v2)
{
    return Vector3<T>(v1.e[0]/v2.e[0], v1.e[1]/v2.e[1], v1.e[2]/v2.e[2]);
}

template <typename T>
COMMON_FUNC inline Vector3<T> operator*(T s, const Vector3<T>& v2)
{
    return Vector3<T>(s*v2.e[0], s*v2.e[1], s*v2.e[2]);
}

template <typename T>
COMMON_FUNC inline Vector3<T> operator/(const Vector3<T>& v1, T s)
{
    return Vector3<T>(v1.e[0]/s, v1.e[1]/s, v1.e[2]/s);
}

template <typename T>
COMMON_FUNC inline Vector3<T> operator*(const Vector3<T>& v1, T s)
{
    return Vector3<T>(v1.e[0]*s, v1.e[1]*s, v1.e[2]*s);
}

template <typename T>
COMMON_FUNC inline T dot(const Vector3<T>& v1, const Vector3<T>& v2)
{
    return v1.e[0]*v2.e[0] + v1.e[1]*v2.e[1] + v1.e[2]*v2.e[2];
}

template <typename T>
COMMON_FUNC inline Vector3<T> cross(const Vector3<T>& v1, const Vector3<T>& v2)
{
    return Vector3<T>((v1.e[1]*v2.e[2] - v1.e[2]*v2.e[1]),
                   (-(v1.e[0]*v2.e[2] - v1.e[2]*v2.e[0])),
                   (v1.e[0]*v2.e[1] - v1.e[1]*v2.e[0]));
}

template <typename T>
COMMON_FUNC inline Vector3<T> unit_vector(const Vector3<T>& v)
{
    return v / v.length();
}

template <typename T>
COMMON_FUNC inline Vector3<T> reflect(const Vector3<T>& v, const Vector3<T>& n)
{
    return v - 2 * dot(v, n) * n;
}

template <typename T>
COMMON_FUNC inline bool refract(const Vector3<T>& v, const Vector3<T>& n, T niOverNt, Vector3<T>& refracted)
{
    Vector3<T> uv = unit_vector(v);
    T dt = dot(uv, n);
    T discriminant = 1 - niOverNt * niOverNt * (1 - dt * dt);
    if (discriminant > 0)
    {
        refracted = niOverNt * (uv - n * dt) - n * Sqrt(discriminant);
        return true;
    }
    return false;
}

typedef Vector3<double> Vector3d;
typedef Vector3<float> Vector3f;

#endif //PATHTRACER_VECTOR3_H
