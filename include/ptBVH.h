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

    COMMON_FUNC bool hit(const Rayf& r, float tmin, float tmax, HitRecord& rec, RNG& rng) const override;
    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override;

    COMMON_FUNC float pdfValue(const Vector3f& o, const Vector3f& v, RNG& rng) const override;
    COMMON_FUNC Vector3f random(const Vector3f& o, RNG& rng) const override;

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream !=nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= left->serialize(pStream);
        ok |= right->serialize(pStream);
        ok |= m_bbox.serialize(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return BVHTypeId; }

private:
    Hitable* left = nullptr;
    Hitable* right = nullptr;
    AABB<float> m_bbox;
};


#endif //PATHTRACER_BVH_H
