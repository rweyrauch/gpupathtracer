/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_VECTOR2_H
#define PATHTRACER_VECTOR2_H

#include <cmath>
#include "ptCudaCommon.h"
#include "ptMath.h"
#include "ptStream.h"

template <typename T>
class Vector2 {
public:
    COMMON_FUNC Vector2() { }

    COMMON_FUNC Vector2(T e0, T e1) :
        e{e0, e1} { }

    COMMON_FUNC inline T x() const { return e[0]; }
    COMMON_FUNC inline T& x() { return e[0]; }

    COMMON_FUNC inline T y() const { return e[1]; }
    COMMON_FUNC inline T& y() { return e[1]; }

    COMMON_FUNC inline T u() const { return e[0]; }
    COMMON_FUNC inline T& u() { return e[0]; }

    COMMON_FUNC inline T v() const { return e[1]; }
    COMMON_FUNC inline T& v() { return e[1]; }

    COMMON_FUNC inline const Vector2& operator+() const { return *this; }

    COMMON_FUNC inline Vector2 operator-() const { return Vector2(-e[0], -e[1]); }

    COMMON_FUNC inline T operator[](int i) const { return e[i]; }

    COMMON_FUNC inline T& operator[](int i) { return e[i]; }

    COMMON_FUNC inline Vector2& operator+=(const Vector2& v2)
    {
        e[0] += v2.e[0];
        e[1] += v2.e[1];
        return *this;
    }

    COMMON_FUNC inline Vector2& operator-=(const Vector2& v2)
    {
        e[0] -= v2.e[0];
        e[1] -= v2.e[1];
        return *this;
    }

    COMMON_FUNC inline Vector2& operator*=(const Vector2& v2)
    {
        e[0] *= v2.e[0];
        e[1] *= v2.e[1];
        return *this;
    }

    COMMON_FUNC inline Vector2& operator/=(const Vector2& v2)
    {
        e[0] /= v2.e[0];
        e[1] /= v2.e[1];
        return *this;
    }

    COMMON_FUNC inline Vector2& operator*=(const T s)
    {
        e[0] *= s;
        e[1] *= s;
        return *this;
    }

    COMMON_FUNC inline Vector2& operator/=(const T s)
    {
        const T invS = 1 / s;
        e[0] *= invS;
        e[1] *= invS;
        return *this;
    }

    COMMON_FUNC inline T length() const
    {
        return Sqrt(squared_length());
    }

    COMMON_FUNC inline T squared_length() const
    {
        return e[0]*e[0] + e[1]*e[1];
    }

    COMMON_FUNC inline void make_unit_vector()
    {
        T k = 1 / length();
        e[0] *= k;
        e[1] *= k;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const
    {
        if (pStream == nullptr)
            return false;

        return pStream->write(e, sizeof(e));
    }

    COMMON_FUNC bool unserialize(Stream* pStream)
    {
        if (pStream == nullptr)
            return false;

        return pStream->read(e, sizeof(e));
    }

    T e[2];
};

template <typename T>
COMMON_FUNC inline Vector2<T> operator+(const Vector2<T>& v1, const Vector2<T>& v2)
{
    return Vector2<T>(v1.e[0]+v2.e[0], v1.e[1]+v2.e[1]);
}

template <typename T>
COMMON_FUNC inline Vector2<T> operator-(const Vector2<T>& v1, const Vector2<T>& v2)
{
    return Vector2<T>(v1.e[0]-v2.e[0], v1.e[1]-v2.e[1]);
}

template <typename T>
COMMON_FUNC inline Vector2<T> operator*(const Vector2<T>& v1, const Vector2<T>& v2)
{
    return Vector2<T>(v1.e[0]*v2.e[0], v1.e[1]*v2.e[1]);
}

template <typename T>
COMMON_FUNC inline Vector2<T> operator/(const Vector2<T>& v1, const Vector2<T>& v2)
{
    return Vector2<T>(v1.e[0]/v2.e[0], v1.e[1]/v2.e[1]);
}

template <typename T>
COMMON_FUNC inline Vector2<T> operator*(T s, const Vector2<T>& v2)
{
    return Vector2<T>(s*v2.e[0], s*v2.e[1]);
}

template <typename T>
COMMON_FUNC inline Vector2<T> operator/(const Vector2<T>& v1, T s)
{
    return Vector2<T>(v1.e[0]/s, v1.e[1]/s);
}

template <typename T>
COMMON_FUNC inline Vector2<T> operator*(const Vector2<T>& v1, T s)
{
    return Vector2<T>(v1.e[0]*s, v1.e[1]*s);
}

template <typename T>
COMMON_FUNC inline T dot(const Vector2<T>& v1, const Vector2<T>& v2)
{
    return v1.e[0]*v2.e[0] + v1.e[1]*v2.e[1];
}

template <typename T>
COMMON_FUNC inline Vector2<T> unit_vector(const Vector2<T>& v)
{
    return v / v.length();
}

template <typename T>
COMMON_FUNC inline Vector2<T> reflect(const Vector2<T>& v, const Vector2<T>& n)
{
    return v - 2 * dot(v, n) * n;
}

template <typename T>
COMMON_FUNC inline bool refract(const Vector2<T>& v, const Vector2<T>& n, T niOverNt, Vector2<T>& refracted)
{
    Vector2<T> uv = unit_vector(v);
    T dt = dot(uv, n);
    T discriminant = 1 - niOverNt * niOverNt * (1 - dt * dt);
    if (discriminant > 0)
    {
        refracted = niOverNt * (uv - n * dt) - n * Sqrt(discriminant);
        return true;
    }
    return false;
}

typedef Vector2<double> Vector2d;
typedef Vector2<float> Vector2f;

#endif //PATHTRACER_VECTOR2_H
