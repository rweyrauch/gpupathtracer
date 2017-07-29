/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_AABB_H
#define PATHTRACER_AABB_H

#include "ptCudaCommon.h"
#include "ptVector3.h"
#include "ptRay.h"
#include "ptStream.h"

template <typename T>
class AABB
{
public:
    COMMON_FUNC AABB() = default;
    COMMON_FUNC AABB(const Vector3<T>& a, const Vector3<T>& b) :
        m_min(a),
        m_max(b) {}

    COMMON_FUNC const Vector3<T>& min() const { return m_min; }
    COMMON_FUNC const Vector3<T>& max() const { return m_max; }

    COMMON_FUNC bool hit(const Ray<T>& r, T tmin, T tmax) const
    {
        for (int a = 0; a < 3; a++)
        {
            const auto invD = 1 / r.direction()[a];
            auto t0 = (m_min[a] - r.origin()[a]) * invD;
            auto t1 = (m_max[a] - r.origin()[a]) * invD;
            if (invD < 0)
            {
                auto temp = t0;
                t0 = t1;
                t1 = temp;
            }
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin) return false;
        }
        return true;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const
    {
        if (pStream == nullptr)
            return false;

        bool ok = m_min.serialize(pStream);
        ok |= m_max.serialize(pStream);

        return ok;
    }

protected:
    Vector3<T> m_min;
    Vector3<T> m_max;
};

template <typename T>
COMMON_FUNC AABB<T> join(const AABB<T>& box0, const AABB<T>& box1)
{
    Vector3<T> small(Min(box0.min().x(), box1.min().x()),
                     Min(box0.min().y(), box1.min().y()),
                     Min(box0.min().z(), box1.min().z()));
    Vector3<T> big(Max(box0.max().x(), box1.max().x()),
                   Max(box0.max().y(), box1.max().y()),
                   Max(box0.max().z(), box1.max().z()));
    return AABB<T>(small, big);
}

#endif //PATHTRACER_AABB_H
