/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_MEDIUM_H
#define PATHTRACER_MEDIUM_H

#include "ptCudaCommon.h"
#include "ptHitable.h"
#include "ptTexture.h"
#include "ptMaterial.h"

class ConstantMedium : public Hitable
{
public:
    COMMON_FUNC ConstantMedium(Hitable* b, float d, Texture* a) :
        boundary(b),
        density(d)
    {
        phaseFunction = new Isotropic(a);
    }

    COMMON_FUNC virtual bool hit(const Rayf& r_in, float t0, float t1, HitRecord& rec) const
    {
        HitRecord rec1, rec2;
        if (boundary->hit(r_in, -FLT_MAX, FLT_MAX, rec1))
        {
            if (boundary->hit(r_in, rec1.t+0.0001, FLT_MAX, rec2))
            {
                if (rec1.t < t0) rec1.t = t0;
                if (rec2.t > t1) rec2.t = t1;
                if (rec1.t > rec2.t) return false;
                if (rec1.t < 0) rec1.t = 0;
                float distInsideBoundary = (rec2.t - rec1.t) * r_in.direction().length();
                float hitDist = -(1/density) * Log(rand(seed0, seed1));
                if (hitDist < distInsideBoundary)
                {
                    rec.t = rec1.t + hitDist / r_in.direction().length();
                    rec.p = r_in.pointAt(rec.t);
                    rec.normal = Vector3f(1, 0, 0);
                    rec.material = phaseFunction;
                    return true;
                }
            }
        }
        return false;
    }

    COMMON_FUNC virtual bool bounds(float t0, float t1, AABB<float>& bbox) const
    {
        return boundary->bounds(t0, t1, bbox);
    }

    Hitable* boundary;
    float density;
    Material* phaseFunction;
};


#endif //PATHTRACER_MEDIUM_H
