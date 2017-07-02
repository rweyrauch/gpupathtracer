/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_ONB_H
#define PATHTRACER_ONB_H

#include "ptCudaCommon.h"
#include "ptVector3.h"

template <typename T>
class ONB
{
public:
    COMMON_FUNC ONB() {}

    COMMON_FUNC Vector3<T> operator[](int i) const { return axis[i]; }
    COMMON_FUNC const Vector3<T>& u() const { return axis[0]; }
    COMMON_FUNC const Vector3<T>& v() const { return axis[1]; }
    COMMON_FUNC const Vector3<T>& w() const { return axis[2]; }

    COMMON_FUNC Vector3<T> local(T a, T b, T c) const { return a * u() + b * v() + c * w(); }
    COMMON_FUNC Vector3<T> local(const Vector3<T>& a) const { return a.x() * u() + a.y() * v() + a.z() * w(); }
    COMMON_FUNC void buildFromW(const Vector3<T>& n)
    {
        axis[2] = unit_vector(n);
        Vector3<T> a;
        if (Abs(w().x()) > static_cast<T>(0.9))
            a = Vector3<T>(0, 1, 0);
        else
            a = Vector3<T>(1, 0, 0);
        axis[1] = unit_vector(cross(w(), a));
        axis[0] = cross(w(), v());
    }

private:
    Vector3<T> axis[3];
};

#endif //PATHTRACER_ONB_H
