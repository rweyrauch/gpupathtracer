/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_RAY_H
#define PATHTRACER_RAY_H

#include "ptCudaCommon.h"
#include "ptVector3.h"

template <typename T>
class Ray {
public:
    COMMON_FUNC Ray() { }

    COMMON_FUNC Ray(const Vector3<T>& o, const Vector3<T>& d, T t = 0) :
        m_origin(o),
        m_dir(d),
        m_time(t) { }

    COMMON_FUNC const Vector3<T>& origin() const { return m_origin; }

    COMMON_FUNC Vector3<T>& origin() { return m_origin; }

    COMMON_FUNC const Vector3<T>& direction() const { return m_dir; }

    COMMON_FUNC Vector3<T>& direction() { return m_dir; }

    COMMON_FUNC T time() const { return m_time; }

    COMMON_FUNC Vector3<T> pointAt(T t) const { return m_origin + t * m_dir; }

protected:
    Vector3<T> m_origin;
    Vector3<T> m_dir;
    T m_time;
};

typedef Ray<float> Rayf;
typedef Ray<double> Rayd;

#endif //PATHTRACER_RAY_H