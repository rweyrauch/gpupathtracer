/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_HITABLELIST_H
#define PATHTRACER_HITABLELIST_H

#include "ptCudaCommon.h"
#include "ptHitable.h"
#include "ptRNG.h"

class HitableList : public Hitable {
public:
    COMMON_FUNC HitableList() {}

    COMMON_FUNC HitableList(int c, Hitable** l) :
        count(c),
        list(l) {}

    COMMON_FUNC bool hit(const Rayf& r, float tmin, float tmax, HitRecord& rec, RNG& rng) const override;

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override;

    COMMON_FUNC float pdfValue(const Vector3f& o, const Vector3f& v, RNG& rng) const override
    {
        float weight = 1 / (float)count;
        float sum = 0;
        for (int i = 0; i < count; i++)
        {
            sum += weight * list[i]->pdfValue(o, v, rng);
        }
        return sum;
    }

    COMMON_FUNC Vector3f random(const Vector3f& o, RNG& rng) const override
    {
        auto index = int(rng.rand() * count);
        return list[index]->random(o, rng);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override;
    COMMON_FUNC bool deserialize(Stream *pStream) override;

    COMMON_FUNC int typeId() const override { return ListTypeId; }

private:
    int count = 0;
    Hitable** list = nullptr;
};

#endif //PATHTRACER_HITABLELIST_H