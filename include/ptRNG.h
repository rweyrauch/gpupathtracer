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
#include "ptCudaCommon.h"
#include "ptVector3.h"

class RNG
{
public:
    COMMON_FUNC RNG() {}

    COMMON_FUNC virtual float rand() = 0;
};

class SimpleRng : public RNG
{
public:
    COMMON_FUNC SimpleRng(unsigned int s0, unsigned int s1) :
        seed0(s0),
        seed1(s1)
    {}

    COMMON_FUNC float rand()
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

    COMMON_FUNC float rand()
    {
#ifndef __CUDA_ARCH__
        return (float)drand48();
#else
        return 0;
#endif
    }
};
#endif //PATHTRACER_RNG_H