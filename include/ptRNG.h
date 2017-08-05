/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */
#ifndef PATHTRACER_RNG_H
#define PATHTRACER_RNG_H

#include <cstdlib>
#include <math_constants.h>
#include <math_functions.h>
#include "ptCudaCommon.h"
#include "ptVector3.h"
#include "ptStream.h"

class RNG
{
public:
    COMMON_FUNC RNG() {}

    COMMON_FUNC virtual float rand() = 0;

    COMMON_FUNC virtual bool serialize(Stream* pStream) const = 0;

};

class SimpleRng : public RNG
{
public:
    COMMON_FUNC SimpleRng(unsigned int s0, unsigned int s1) :
        seed0(s0),
        seed1(s1)
    {}

    COMMON_FUNC float rand() override
    {
        seed0 = 36969 * ((seed0) & 65535) + ((seed0) >> 16);  // hash the seeds using bitwise AND and bitshifts
        seed1 = 18000 * ((seed1) & 65535) + ((seed1) >> 16);

        unsigned int ires = ((seed0) << 16) + (seed1);

        // Convert to float
        union {
            float f;
            unsigned int ui;
        } res;

        res.ui = (ires & 0x007fffff) | 0x40000000;  // bitwise AND, bitwise OR

        return ((res.f - 2.0f) / 2.0f);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        return false;
    }

private:
    unsigned int seed0, seed1;
};

class DRandRng : public RNG
{
public:
    COMMON_FUNC DRandRng(long int seed)
    {
#ifndef __CUDA_ARCH__
        srand48(seed);
#endif
    }

    COMMON_FUNC float rand() override
    {
#ifndef __CUDA_ARCH__
        return (float)drand48();
#else
        return 0;
#endif
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        return false;
    }
};

/*!
 * PCG random number generator.
 *
 * http://www.pcg-random.org/
 *
 */
class PcgRng : public RNG
{
    const float OneMinusEpsilon = 0.99999994f;

public:
    COMMON_FUNC PcgRng(uint64_t seed)
    {
        state = 0u;
        inc = (seed << 1u) | 1u;
        uniformUInt32();
        state += 0x853c49e6748fea9bULL;
        uniformUInt32();
    }

    COMMON_FUNC float rand() override
    {
        return Min(OneMinusEpsilon, (uniformUInt32() * 2.3283064365386963e-10f));
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        return false;
    }

private:

    COMMON_FUNC uint32_t uniformUInt32()
    {
        uint64_t oldstate = state;
        // Advance internal state
        state = oldstate * 0x5851f42d4c957f2dULL + inc;
        // Calculate output function (XSH RR), uses old state for max ILP
        uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
        uint32_t rot = (uint32_t)(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
    }
private:
    uint64_t state, inc;
};


COMMON_FUNC inline Vector3f randomInUnitSphere(RNG& rng)
{
    const float phi = rng.rand() * 2 * CUDART_PI_F;
    const float z = 1 - 2 * rng.rand();
    const float r = Sqrt(Max(0, 1 - z * z));
    return Vector3f(r * Cos(phi), r * Sin(phi), z);
}

COMMON_FUNC inline Vector3f randomInUnitDisk(RNG& rng)
{
    const float r = Sqrt(rng.rand());
    const float theta = rng.rand() * 2 * CUDART_PI_F;
    return Vector3f(r * Cos(theta), r * Sin(theta), 0);
}

COMMON_FUNC inline Vector3f randomCosineDirection(RNG& rng)
{
    float r1 = rng.rand();
    float r2 = rng.rand();
    float z = Sqrt(1 - r2);
    float phi = 2 * CUDART_PI_F * r1;
    float x = Cos(phi) * 2 * Sqrt(r2);
    float y = Sin(phi) * 2 * Sqrt(r2);
    return Vector3f(x, y, z);
}

COMMON_FUNC inline Vector3f randomToUnitSphere(float radius, float distSqrd, RNG& rng)
{
    float r1 = rng.rand();
    float r2 = rng.rand();
    float z = 1 + r2 * (Sqrt(1 - radius*radius/distSqrd) - 1);
    float phi = 2 * CUDART_PI_F * r1;
    float x = Cos(phi) * Sqrt(1-z*z);
    float y = Sin(phi) * Sqrt(1-z*z);
    return Vector3f(x, y, z);
}

COMMON_FUNC inline float rand(unsigned int *seed0, unsigned int *seed1)
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

#endif //PATHTRACER_RNG_H