/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_TEXTURE_H
#define PATHTRACER_TEXTURE_H

#include "ptCudaCommon.h"
#include "ptVector2.h"
#include "ptVector3.h"

class Texture
{
public:
    COMMON_FUNC virtual Vector3f value(const Vector2f& uv, const Vector3f& p) const = 0;
};

class ConstantTexture : public Texture
{
public:
    COMMON_FUNC ConstantTexture()
    {}

    COMMON_FUNC ConstantTexture(const Vector3f& c) :
        color(c)
    {}

    COMMON_FUNC virtual Vector3f value(const Vector2f& uv, const Vector3f& p) const
    {
        return color;
    }

    Vector3f color;
};

class CheckerTexture : public Texture
{
public:
    COMMON_FUNC CheckerTexture()
    {}

    COMMON_FUNC CheckerTexture(Texture* t0, Texture* t1) :
        odd(t1),
        even(t0)
    {}

    COMMON_FUNC virtual Vector3f value(const Vector2f& uv, const Vector3f& p) const
    {
        float sines = Sin(scaler * p.x()) * Sin(scaler * p.y()) * Sin(scaler * p.z());
        if (sines < 0)
            return odd->value(uv, p);
        else
            return even->value(uv, p);
    }

private:
    float scaler = 10;
    Texture *odd = nullptr;
    Texture *even = nullptr;
};

#endif //PATHTRACER_TEXTURE_H
