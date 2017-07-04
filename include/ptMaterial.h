/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_MATERIAL_H
#define PATHTRACER_MATERIAL_H

#include <math_constants.h>
#include "ptCudaCommon.h"
#include "ptPDF.h"
#include "ptVector3.h"
#include "ptRay.h"
#include "ptHitable.h"
#include "ptTexture.h"

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

COMMON_FUNC inline Vector3f randomInUnitSphere(unsigned int *seed0, unsigned int *seed1)
{
    const float phi = rand(seed0, seed1) * 2.0f * CUDART_PI_F;
    const float z = 1.0f - 2.0f * rand(seed0, seed1);
    const float r = sqrtf(fmaxf(0.0f, 1.0f - z * z));
    return Vector3f(r * cosf(phi), r * sinf(phi), z);
}

COMMON_FUNC inline Vector3f randomInUnitDisk(unsigned int *seed0, unsigned int *seed1)
{
    const float r = sqrtf(rand(seed0, seed1));
    const float theta = rand(seed0, seed1) * 2.0f * CUDART_PI_F;
    return Vector3f(r * cosf(theta), r * sinf(theta), 0.0f);
}

template <typename T>
COMMON_FUNC inline T schlick(T cs, T ri)
{
    T r0 = (1 - ri) / (1 + ri);
    r0 = r0 * r0;
    return r0 + (1 - r0) * Pow((1 - cs), 5);
}

class Material
{
public:
    COMMON_FUNC virtual ~Material() {}

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, Vector3f& attenuation, Rayf& scattered, unsigned int *seed0, unsigned int *seed1) const = 0;
    COMMON_FUNC virtual Vector3f emitted(const Vector2f& uv, const Vector3f& p) const { return Vector3f(0.0f, 0.0f, 0.0f); }
};

class Lambertian : public Material
{
public:
    COMMON_FUNC Lambertian(Texture* a) :
        albedo(a) { }

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, Vector3f& attenuation, Rayf& scattered, unsigned int *seed0, unsigned int *seed1) const
    {
        Vector3f target = rec.p + rec.normal * randomInUnitSphere(seed0, seed1);
        scattered = Rayf(rec.p, target-rec.p);
        attenuation = albedo->value(rec.uv, rec.p);
        return true;
    }

private:
    Texture* albedo;
};

class Metal : public Material
{
public:
    COMMON_FUNC Metal(const Vector3f& a, float f) :
        albedo(a),
        fuzz(1)
    {
        if (f < 1) { fuzz = f; }
    }

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, Vector3f& attenuation, Rayf& scattered, unsigned int *seed0, unsigned int *seed1) const
    {
        Vector3f reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = Rayf(rec.p, reflected+fuzz*randomInUnitSphere(seed0, seed1));
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0.0f);
    }

private:
    Vector3f albedo;
    float fuzz;
};

class Dielectric : public Material
{
public:
    COMMON_FUNC Dielectric(float ri) :
        refIndex(ri) { }

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, Vector3f& attenuation, Rayf& scattered, unsigned int *seed0, unsigned int *seed1) const
    {
        Vector3f outwardNormal;
        Vector3f reflected = reflect(r_in.direction(), rec.normal);
        float niOverNt;
        attenuation = Vector3f(1.0f, 1.0f, 1.0f);
        Vector3f refracted;
        float reflectProb, cosine;
        if (dot(r_in.direction(), rec.normal) > 0.0f)
        {
            outwardNormal = -rec.normal;
            niOverNt = refIndex;
            cosine = refIndex * dot(r_in.direction(), rec.normal) / r_in.direction().length();
        }
        else
        {
            outwardNormal = rec.normal;
            niOverNt = 1.0f / refIndex;
            cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
        }
        if (refract(r_in.direction(), outwardNormal, niOverNt, refracted))
        {
            reflectProb = schlick(cosine, refIndex);
        }
        else
        {
            reflectProb = 1.0f;
        }
        if (rand(seed0, seed1) < reflectProb)
        {
            scattered = Rayf(rec.p, reflected);
        }
        else
        {
            scattered = Rayf(rec.p, refracted);
        }
        return true;
    }

private:
    float refIndex;
};

class DiffuseLight : public Material
{
public:
    COMMON_FUNC DiffuseLight(Texture* a) :
        emit(a) {}

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, Vector3f& attenuation, Rayf& scattered, unsigned int *seed0, unsigned int *seed1) const
    {
        return false;
    }
    COMMON_FUNC virtual Vector3f emitted(const Vector2f& uv, const Vector3f& p) const
    {
        return emit->value(uv, p);
    }

private:
    Texture* emit;
};

#endif //PATHTRACER_MATERIAL_H