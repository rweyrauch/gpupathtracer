/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_MATH_H
#define PATHTRACER_MATH_H

#include <cmath>
#include "ptCudaCommon.h"

template<typename T>
COMMON_FUNC inline T Clamp(T val, T min, T max)
{
    return ((val > max) ? max : ((val < min) ? min : val));
}

COMMON_FUNC inline float Sqrt(float val) { return sqrtf(val); }
COMMON_FUNC inline double Sqrt(double val) { return sqrt(val); }

COMMON_FUNC inline float Max(float v0, float v1) { return fmaxf(v0, v1); }
COMMON_FUNC inline double Max(double v0, double v1) { return fmax(v0, v1); }

COMMON_FUNC inline float Min(float v0, float v1) { return fminf(v0, v1); }
COMMON_FUNC inline double Min(double v0, double v1) { return fmin(v0, v1); }

COMMON_FUNC inline float Abs(float v0) { return fabsf(v0); }
COMMON_FUNC inline double Abs(double v0) { return fabs(v0); }

COMMON_FUNC inline float Sin(float v0) { return sinf(v0); }
COMMON_FUNC inline double Sin(double v0) { return sin(v0); }
COMMON_FUNC inline float Cos(float v0) { return cosf(v0); }
COMMON_FUNC inline double Cos(double v0) { return cos(v0); }
COMMON_FUNC inline float Tan(float v0) { return tanf(v0); }
COMMON_FUNC inline double Tan(double v0) { return tan(v0); }

COMMON_FUNC inline float Log(float v0) { return logf(v0); }
COMMON_FUNC inline double Log(double v0) { return log(v0); }

COMMON_FUNC inline float Pow(float x, float y) { return powf(x, y); }
COMMON_FUNC inline double Pow(double x, double y) { return pow(x, y); }

#endif //PATHTRACER_MATH_H