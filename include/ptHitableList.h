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
    COMMON_FUNC HitableList() = default;

    COMMON_FUNC HitableList(int c, Hitable** l) :
        count(c),
        list(l) {}

    COMMON_FUNC bool hit(const Rayf& r, float tmin, float tmax, HitRecord& rec, RNG& rng) const override
    {
        HitRecord temp_rec;
        bool hit_anything = false;
        float closest_so_far = tmax;
        for (int i = 0; i < count; i++)
        {
            if (list[i]->hit(r, tmin, closest_so_far, temp_rec, rng))
            {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override
    {
        if (count == 0) return false;

        AABB<float> tempBox;
        bool first = list[0]->bounds(t0, t1, tempBox);
        if (!first)
            return false;

        bbox = tempBox;

        for (int i = 0; i < count; i++)
        {
            if (list[i]->bounds(t0, t1, tempBox))
            {
                bbox = join(bbox, tempBox);
            }
            else
                return false;
        }
        return true;
    }

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

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= pStream->write(&count, sizeof(count));
        for (int i = 0; i < count; i++)
        {
            ok |= list[i]->serialize(pStream);
        }
        return ok;
    }

    COMMON_FUNC int typeId() const override { return ListTypeId; }

private:
    int count = 0;
    Hitable** list = nullptr;
};

#endif //PATHTRACER_HITABLELIST_H