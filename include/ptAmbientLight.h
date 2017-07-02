/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_AMBIENTLIGHT_H
#define PATHTRACER_AMBIENTLIGHT_H

#include "ptCudaCommon.h"
#include "ptVector3.h"
#include "ptRay.h"

class AmbientLight
{
public:
    COMMON_FUNC virtual Vector3f emitted(const Rayf& ray) const = 0;
};

class ConstantAmbient : public AmbientLight
{
public:
    COMMON_FUNC ConstantAmbient() :
        color(0, 0, 0) {}

    COMMON_FUNC ConstantAmbient(const Vector3f& c) :
        color(c) {}

    COMMON_FUNC virtual Vector3f emitted(const Rayf& ray) const
    {
        return color;
    }

private:
    Vector3f color;
};

class SkyAmbient : public AmbientLight
{
public:
    COMMON_FUNC SkyAmbient() {}

    COMMON_FUNC virtual Vector3f emitted(const Rayf& ray) const
    {
        Vector3f unit_dir = unit_vector(ray.direction());
        float t = 0.5f * (unit_dir.y() + 1);
        return (1 - t) * Vector3f(1, 1, 1) + t * Vector3f(0.5f, 0.7f, 1.0f);
    }
};

#endif //PATHTRACER_AMBIENTLIGHT_H
