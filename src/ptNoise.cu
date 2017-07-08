/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptNoise.h"
#include "ptVector3.h"

//
// JAVA REFERENCE IMPLEMENTATION OF IMPROVED NOISE - COPYRIGHT 2002 KEN PERLIN.
//
// Ported to C++.
//

template<typename T>
COMMON_FUNC T fade(T t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

template<typename T>
COMMON_FUNC T lerp(T t, T a, T b)
{
    return a + t * (b - a);
}

template<typename T>
COMMON_FUNC T grad(int hash, T x, T y, T z)
{
    int h = hash & 15;                             // CONVERT LO 4 BITS OF HASH CODE
    T u = ((h<8) || (h==12) || (h==13)) ? x : y;   // INTO 12 GRADIENT DIRECTIONS.
    T v = ((h<4) || (h==12) || (h==13)) ? y : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

#ifdef __CUDA_ARCH__
__device__ const unsigned int NUM_NOISE_PERM = 256;
__device__ unsigned char gPermutations[NUM_NOISE_PERM + NUM_NOISE_PERM] =
#else
static const unsigned int NUM_NOISE_PERM = 256;
static unsigned char gPermutations[NUM_NOISE_PERM + NUM_NOISE_PERM] =
#endif
{
    151, 160, 137, 91, 90, 15, 131, 13,
    201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99,
    37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62,
    94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87,
    174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166,
    77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41,
    55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73,
    209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86,
    164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202,
    38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17,
    182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70,
    221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110,
    79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193,
    238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107,
    49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45,
    127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141,
    128, 195, 78, 66, 215, 61, 156, 180,
    151, 160, 137, 91, 90, 15, 131, 13,
    201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99,
    37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62,
    94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87,
    174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166,
    77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41,
    55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73,
    209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86,
    164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202,
    38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17,
    182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70,
    221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110,
    79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193,
    238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107,
    49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45,
    127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141,
    128, 195, 78, 66, 215, 61, 156, 180
};

COMMON_FUNC float Noise(const Vector3f& p)
{
    int xi = (int)floorf(p.x()) & 255;       // FIND UNIT CUBE THAT
    int yi = (int)floorf(p.y()) & 255;       // CONTAINS POINT.
    int zi = (int)floorf(p.z()) & 255;
    auto x = p.x() - floorf(p.x());          // FIND RELATIVE X,Y,Z
    auto y = p.y() - floorf(p.y());          // OF POINT IN CUBE.
    auto z = p.z() - floorf(p.z());
    float u = fade(x);                     // COMPUTE FADE CURVES
    float v = fade(y);                     // FOR EACH OF X,Y,Z.
    float w = fade(z);
    int A =  gPermutations[xi]+yi;          // HASH COORDINATES OF
    int AA = gPermutations[A]+zi;           // THE 8 CUBE CORNERS,
    int AB = gPermutations[A+1]+zi;
    int B =  gPermutations[xi+1]+yi;
    int BA = gPermutations[B]+zi;
    int BB = gPermutations[B+1]+zi;

    return lerp(w, lerp(v, lerp(u, grad(gPermutations[AA], x, y, z),    // AND ADD
                                grad(gPermutations[BA], x-1, y, z)),    // BLENDED
                        lerp(u, grad(gPermutations[AB], x, y-1, z),     // RESULTS
                             grad(gPermutations[BB], x-1, y-1, z))),    // FROM  8
                lerp(v, lerp(u, grad(gPermutations[AA+1], x, y, z-1),   // CORNERS
                             grad(gPermutations[BA+1], x-1, y, z-1)),   // OF CUBE
                     lerp(u, grad(gPermutations[AB+1], x, y-1, z-1),
                          grad(gPermutations[BB+1], x-1, y-1, z-1))));
}

COMMON_FUNC float Turbulence(const Vector3f& p, int depth)
{
    float accum = 0;
    Vector3f temp_p = p;
    float weight = 1;
    for (int i = 0; i < depth; i++)
    {
        accum += weight * Noise(temp_p);
        weight *= 0.5f;
        temp_p *= 2.0f;
    }
    return fabsf(accum);
}
