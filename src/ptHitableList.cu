/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptHitableList.h"

bool HitableList::hit(const Rayf &r, float tmin, float tmax, HitRecord &rec, RNG &rng) const
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

bool HitableList::bounds(float t0, float t1, AABB<float> &bbox) const
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

bool HitableList::serialize(Stream *pStream) const
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

bool HitableList::deserialize(Stream *pStream)
{
    if (pStream == nullptr)
        return false;

    bool ok = pStream->read(&count, sizeof(count));
    if (ok && (count > 0))
    {
        list = new Hitable*[count];
        for (int i = 0; i < count; i++)
        {
            list[i] = Hitable::Create(pStream);
        }
    }

    return ok;
}
