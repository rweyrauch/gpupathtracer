/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_PDF_H
#define PATHTRACER_PDF_H

#include <cmath>
#include "ptVector3.h"
#include "ptONB.h"
#include "ptRNG.h"
#include "ptHitable.h"

COMMON_FUNC float rand(unsigned int *seed0, unsigned int *seed1)
{
    *seed0 = 36969 * ((*seed0) & 65535) + ((*seed0) >> 16);  // hash the seeds using bitwise AND and bitshifts
    *seed1 = 18000 * ((*seed1) & 65535) + ((*seed1) >> 16);

    unsigned int ires = ((*seed0) << 16) + (*seed1);

    // Convert to float
    union {
        float f;
        unsigned int ui;
    } res;

    res.ui = (ires & 0x007fffff) | 0x40000000;  // bitwise AND, bitwise OR

    return (res.f - 2.0f) / 2.0f;
}

COMMON_FUNC inline Vector3f randomInUnitSphere(RNG* rng)
{
    const float phi = rng->rand() * 2 * M_PI;
    const float z = 1 - 2 * rng->rand();
    const float r = Sqrt(Max(0, 1 - z * z));
    return Vector3f(r * Cos(phi), r * Sin(phi), z);
}

COMMON_FUNC inline Vector3f randomInUnitDisk(RNG* rng)
{
    const float r = Sqrt(rng->rand());
    const float theta = rng->rand() * 2 * M_PI;
    return Vector3f(r * Cos(theta), r * Sin(theta), 0);
}

COMMON_FUNC inline Vector3f randomCosineDirection(RNG* rng)
{
    float r1 = rng->rand();
    float r2 = rng->rand();
    float z = Sqrt(1 - r2);
    float phi = 2 * M_PI * r1;
    float x = Cos(phi) * 2 * Sqrt(r2);
    float y = Sin(phi) * 2 * Sqrt(r2);
    return Vector3f(x, y, z);
}

COMMON_FUNC inline Vector3f randomToUnitSphere(float radius, float distSqrd, RNG* rng)
{
    float r1 = rng->rand();
    float r2 = rng->rand();
    float z = 1 + r2 * (Sqrt(1 - radius*radius/distSqrd) - 1);
    float phi = 2 * M_PI * r1;
    float x = Cos(phi) * Sqrt(1-z*z);
    float y = Sin(phi) * Sqrt(1-z*z);
    return Vector3f(x, y, z);
}

class Pdf
{
public:
    COMMON_FUNC virtual ~Pdf() {}
    COMMON_FUNC virtual float value(const Vector3f& direction) const = 0;
    COMMON_FUNC virtual Vector3f generate(RNG* rng) const = 0;
};

class ConstPdf : public Pdf
{
public:
    COMMON_FUNC virtual float value(const Vector3f& direction) const { return 1; };
    COMMON_FUNC virtual Vector3f generate(RNG* rng) const { return randomToUnitSphere(1, 1, rng); };
};

class CosinePdf : public Pdf
{
public:
    COMMON_FUNC CosinePdf(const Vector3f& w) { uvw.buildFromW(w); }

    COMMON_FUNC virtual float value(const Vector3f& direction) const
    {
        float cosine = dot(unit_vector(direction), uvw.w());
        if (cosine > 0)
            return cosine / M_PI;
        else
            return 0;
    }
    COMMON_FUNC virtual Vector3f generate(RNG* rng) const
    {
        return uvw.local(randomCosineDirection(rng));
    }

private:
    ONB<float> uvw;
};

class HitablePdf : public Pdf
{
public:
    COMMON_FUNC HitablePdf(Hitable* p, const Vector3f& o) :
        origin(o),
        hitable(p) {}

    COMMON_FUNC virtual float value(const Vector3f& direction) const
    {
        return hitable->pdfValue(origin, direction);
    }
    COMMON_FUNC virtual Vector3f generate(RNG* rng) const
    {
        return hitable->random(origin, rng);
    }

private:
    Vector3f origin;
    Hitable* hitable;
};

class MixturePdf : public Pdf
{
public:
    COMMON_FUNC MixturePdf(Pdf* p0, Pdf* p1) { p[0] = p0; p[1] = p1; }

    COMMON_FUNC virtual float value(const Vector3f& direction) const
    {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    COMMON_FUNC virtual Vector3f generate(RNG* rng) const
    {
        if (rng->rand() < 0.5)
            return p[0]->generate(rng);
        else
            return p[1]->generate(rng);
    }

private:
    Pdf* p[2];
};
#endif //PATHTRACER_PDF_H
