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

class HitableList : public Hitable {
public:
    COMMON_FUNC HitableList() { }

    COMMON_FUNC HitableList(int c, Hitable** l) :
        count(c),
        list(l) {}

    COMMON_FUNC virtual bool hit(const Rayf& r, float tmin, float tmax, HitRecord& rec) const
    {
        HitRecord temp_rec;
        bool hit_anything = false;
        float closest_so_far = tmax;
        for (int i = 0; i < count; i++)
        {
            if (list[i]->hit(r, tmin, closest_so_far, temp_rec))
            {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }

    int count;
    Hitable** list;
};

#endif //PATHTRACER_HITABLELIST_H