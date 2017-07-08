/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_NOISE_H
#define PATHTRACER_NOISE_H

#include "ptCudaCommon.h"
#include "ptVector3.h"

COMMON_FUNC float Noise(const Vector3f& p);
COMMON_FUNC float Turbulence(const Vector3f& p, int depth=7);

#endif //PATHTRACER_NOISE_H
