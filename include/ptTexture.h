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
#include "ptNoise.h"

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

class NoiseTexture : public Texture
{
public:
    COMMON_FUNC NoiseTexture(float sc) :
        scale(sc)
    {}

    COMMON_FUNC virtual Vector3f value(const Vector2f& uv, const Vector3f &p) const
    {
        float n = 0.5f * (1 + sinf(scale * p.z() + 10 * Turbulence(p)));
        return Vector3f(n, n, n);
    }

    float scale = 1.0f;
};

class ImageTexture : public Texture
{
public:
    COMMON_FUNC ImageTexture()
    {}

    COMMON_FUNC ImageTexture(const unsigned char *pixels, int Nx, int Ny) :
        data(pixels),
        nx(Nx),
        ny(Ny)
    {}

    COMMON_FUNC virtual Vector3f value(const Vector2f& uv, const Vector3f& p) const
    {
        int i = uv.u() * nx;
        int j = (1 - uv.v()) * ny - 0.001f;
        i = max(0, i);
        j = max(0, j);
        i = min(i, nx-1);
        j = min(j, ny-1);
        float r = int(data[3*i + 3*nx*j + 0]) / 255.0f;
        float g = int(data[3*i + 3*nx*j + 1]) / 255.0f;
        float b = int(data[3*i + 3*nx*j + 2]) / 255.0f;
        return Vector3f(r, g, b);
    }

    const unsigned char *data;
    int nx, ny;
};

#endif //PATHTRACER_TEXTURE_H
