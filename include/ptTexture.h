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

#include <math_functions.h>
#include "ptCudaCommon.h"
#include "ptVector2.h"
#include "ptVector3.h"
#include "ptNoise.h"

enum TextureTypeId
{
  ConstantTextureTypeId,
  CheckerTextureTypeId,
  NoiseTextureTypeId,
  ImageTextureTypeId
};

class Texture
{
public:
    COMMON_FUNC virtual Vector3f value(const Vector2f& uv, const Vector3f& p) const = 0;
    COMMON_FUNC virtual bool serialize(Stream* pStream) const = 0;
    COMMON_FUNC virtual bool unserialize(Stream* pStream) = 0;
    COMMON_FUNC virtual int typeId() const = 0;

    COMMON_FUNC static Texture* Create(Stream* pStream);
};

class ConstantTexture : public Texture
{
public:
    COMMON_FUNC ConstantTexture() = default;

    COMMON_FUNC explicit ConstantTexture(const Vector3f& c) :
        color(c)
    {}

    COMMON_FUNC Vector3f value(const Vector2f& uv, const Vector3f& p) const override
    {
        return color;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= color.serialize(pStream);

        return ok;
    }

    COMMON_FUNC bool unserialize(Stream* pStream) override
    {
        if (pStream == nullptr)
            return false;

        bool ok = color.unserialize(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return ConstantTextureTypeId; }

private:
    Vector3f color;
};

class CheckerTexture : public Texture
{
public:
    COMMON_FUNC CheckerTexture() = default;

    COMMON_FUNC CheckerTexture(Texture* t0, Texture* t1) :
        odd(t1),
        even(t0)
    {}

    COMMON_FUNC Vector3f value(const Vector2f& uv, const Vector3f& p) const override
    {
        float sines = Sin(scaler * p.x()) * Sin(scaler * p.y()) * Sin(scaler * p.z());
        if (sines < 0)
            return odd->value(uv, p);
        else
            return even->value(uv, p);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= pStream->write(&scaler, sizeof(scaler));
        ok |= odd->serialize(pStream);
        ok |= even->serialize(pStream);

        return ok;
    }

    COMMON_FUNC bool unserialize(Stream* pStream) override
    {
        if (pStream == nullptr)
            return false;

        bool ok = pStream->read(&scaler, sizeof(scaler));
        odd = Texture::Create(pStream);
        even = Texture::Create(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return CheckerTextureTypeId; }

private:
    float scaler = 10;
    Texture *odd = nullptr;
    Texture *even = nullptr;
};

class NoiseTexture : public Texture
{
public:
    COMMON_FUNC explicit NoiseTexture(float sc) :
        scale(sc)
    {}

    COMMON_FUNC Vector3f value(const Vector2f& uv, const Vector3f &p) const override
    {
        float n = 0.5f * (1 + sinf(scale * p.z() + 10 * Turbulence(p)));
        return Vector3f(n, n, n);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= pStream->write(&scale, sizeof(scale));

        return ok;
    }

    COMMON_FUNC bool unserialize(Stream* pStream) override
    {
        if (pStream == nullptr)
            return false;

        bool ok = pStream->read(&scale, sizeof(scale));

        return ok;
    }

    COMMON_FUNC int typeId() const override { return NoiseTextureTypeId; }

private:
    float scale = 1.0f;
};

class ImageTexture : public Texture
{
public:
    COMMON_FUNC ImageTexture() = default;

    COMMON_FUNC ImageTexture(unsigned char *pixels, int Nx, int Ny) :
        data(pixels),
        nx(Nx),
        ny(Ny)
    {}

    COMMON_FUNC Vector3f value(const Vector2f& uv, const Vector3f& p) const override
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

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= pStream->write(&nx, sizeof(nx));
        ok |= pStream->write(&ny, sizeof(ny));
        ok |= pStream->write(data, nx * ny * sizeof(unsigned char));

        return ok;
    }

    COMMON_FUNC bool unserialize(Stream* pStream) override
    {
        if (pStream == nullptr)
            return false;

        bool ok = pStream->read(&nx, sizeof(nx));
        ok |= pStream->read(&ny, sizeof(ny));

        delete[] data;
        data = new unsigned char[nx * ny];
        ok |= pStream->read(data, nx * ny * sizeof(unsigned char));

        return ok;
    }

    COMMON_FUNC int typeId() const override { return ImageTextureTypeId; }

private:
    unsigned char *data;
    int nx, ny;
};

#endif //PATHTRACER_TEXTURE_H
