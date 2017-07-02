/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_CUDACOMMON_H
#define PATHTRACER_CUDACOMMON_H

#include <cuda.h>

#ifdef __CUDACC__
#define COMMON_FUNC __host__ __device__
#else
#define COMMON_FUNC
#endif

inline int IDIVUP(int numer, int denom)
{
    return ((numer) % (denom) != 0) ? ((numer) / (denom) + 1) : ((numer) / (denom));
}

#endif // PATHTRACER_CUDACOMMON_H