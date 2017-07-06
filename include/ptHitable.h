/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_HITABLE_H
#define PATHTRACER_HITABLE_H

#include "ptCudaCommon.h"
#include "ptVector3.h"
#include "ptVector2.h"
#include "ptRay.h"
#include "ptAABB.h"

class Material;
class RNG;

struct HitRecord
{
    float t;
    Vector3f p;
    Vector3f normal;
    Material* material;
    Vector2f uv;
};

class Hitable
{
public:
    COMMON_FUNC virtual ~Hitable() {}
    COMMON_FUNC virtual bool hit(const Rayf& r, float t_min, float t_max, HitRecord& rec) const = 0;
    COMMON_FUNC virtual bool bounds(float t0, float t1, AABB<float>& bbox) const = 0;

};

#endif //PATHTRACER_HITABLE_H