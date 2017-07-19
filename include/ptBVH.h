/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_BVH_H
#define PATHTRACER_BVH_H

#include "ptCudaCommon.h"
#include "ptHitable.h"
#include "ptAABB.h"

class BVH : public Hitable
{
public:
    COMMON_FUNC BVH(Hitable** list, int length, float time0, float time1, RNG& rng);

    COMMON_FUNC virtual bool hit(const Rayf& r, float tmin, float tmax, HitRecord& rec) const;
    COMMON_FUNC virtual bool bounds(float t0, float t1, AABB<float>& bbox) const;

    COMMON_FUNC virtual float pdfValue(const Vector3f& o, const Vector3f& v) const;
    COMMON_FUNC virtual Vector3f random(const Vector3f& o, RNG& rng) const;

private:
    Hitable* left;
    Hitable* right;
    AABB<float> m_bbox;
};


#endif //PATHTRACER_BVH_H
