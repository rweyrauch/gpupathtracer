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

#include <cfloat>
#include "ptCudaCommon.h"
#include "ptHitable.h"
#include "ptTexture.h"
#include "ptMaterial.h"

class ConstantMedium : public Hitable
{
public:
    COMMON_FUNC ConstantMedium() = default;

    COMMON_FUNC ConstantMedium(Hitable* b, float d, Texture* a) :
        boundary(b),
        density(d)
    {
        phaseFunction = new Isotropic(a);
    }

    COMMON_FUNC bool hit(const Rayf& r_in, float t0, float t1, HitRecord& rec, RNG& rng) const override
    {
        HitRecord rec1, rec2;
        if (boundary->hit(r_in, -FLT_MAX, FLT_MAX, rec1, rng))
        {
            if (boundary->hit(r_in, rec1.t+0.0001, FLT_MAX, rec2, rng))
            {
                if (rec1.t < t0) rec1.t = t0;
                if (rec2.t > t1) rec2.t = t1;
                if (rec1.t > rec2.t) return false;
                if (rec1.t < 0) rec1.t = 0;
                float distInsideBoundary = (rec2.t - rec1.t) * r_in.direction().length();
                float hitDist = -(1/density) * Log(rng.rand());
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

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override
    {
        return boundary->bounds(t0, t1, bbox);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= boundary->serialize(pStream);
        ok |= pStream->write(&density, sizeof(density));
        ok |= phaseFunction->serialize(pStream);

        return ok;
    }

    COMMON_FUNC bool unserialize(Stream* pStream) override
    {
        if (pStream == nullptr)
            return false;

        boundary = Hitable::Create(pStream);
        bool ok = pStream->read(&density, sizeof(density));
        phaseFunction = Material::Create(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return MediumTypeId; }

private:
    Hitable* boundary;
    float density;
    Material* phaseFunction;
};


#endif //PATHTRACER_MEDIUM_H
