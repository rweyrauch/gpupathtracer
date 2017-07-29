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
#include <math_constants.h>
#include "ptVector3.h"
#include "ptONB.h"
#include "ptRNG.h"
#include "ptHitable.h"

class Pdf
{
public:
    COMMON_FUNC virtual ~Pdf() {}
    COMMON_FUNC virtual float value(const Vector3f& direction, RNG& rng) const = 0;
    COMMON_FUNC virtual Vector3f generate(RNG& rng) const = 0;
};

class ConstPdf : public Pdf
{
public:
    COMMON_FUNC float value(const Vector3f& direction, RNG& rng) const override
    {
        return 1;
    };
    COMMON_FUNC Vector3f generate(RNG& rng) const override
    {
        return randomToUnitSphere(1, 1, rng);
    };
};

class CosinePdf : public Pdf
{
public:
    COMMON_FUNC CosinePdf(const Vector3f& w) { uvw.buildFromW(w); }

    COMMON_FUNC float value(const Vector3f& direction, RNG& rng) const override
    {
        float cosine = dot(unit_vector(direction), uvw.w());
        if (cosine > 0)
            return cosine / CUDART_PI_F;
        else
            return 0;
    }
    COMMON_FUNC Vector3f generate(RNG& rng) const override
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

    COMMON_FUNC float value(const Vector3f& direction, RNG& rng) const override
    {
        return hitable->pdfValue(origin, direction, rng);
    }
    COMMON_FUNC Vector3f generate(RNG& rng) const override
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

    COMMON_FUNC float value(const Vector3f& direction, RNG& rng) const override
    {
        return 0.5f * p[0]->value(direction, rng) + 0.5f * p[1]->value(direction, rng);
    }

    COMMON_FUNC Vector3f generate(RNG& rng) const override
    {
        if (rng.rand() < 0.5f)
            return p[0]->generate(rng);
        else
            return p[1]->generate(rng);
    }

private:
    Pdf* p[2];
};

#endif //PATHTRACER_PDF_H
