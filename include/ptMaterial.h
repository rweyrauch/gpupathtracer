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
#include "ptRNG.h"
#include "ptVector3.h"
#include "ptRay.h"
#include "ptHitable.h"
#include "ptTexture.h"
#include "ptPDF.h"

struct ScatterRecord
{
    Rayf specularRay;
    bool isSpecular;
    Vector3f attenuation;
    //Pdf* pdf;
    bool cosinePdf;
};

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

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const = 0;
    COMMON_FUNC virtual float scatteringPdf(const Rayf& r_in, const HitRecord& rec, const Rayf& scattered) const { return 0; }
    COMMON_FUNC virtual Vector3f emitted(const Rayf& r_in, const HitRecord& rec, const Vector2f& uv, const Vector3f& p) const { return Vector3f(0, 0, 0); }
};

class Lambertian : public Material
{
public:
    COMMON_FUNC Lambertian(Texture* a) :
        albedo(a) { }

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const
    {
        srec.isSpecular = false;
        srec.attenuation = albedo->value(rec.uv, rec.p);
        //srec.pdf = new CosinePdf(rec.normal);
        srec.cosinePdf = true;
        return true;
    }

    COMMON_FUNC virtual float scatteringPdf(const Rayf& r_in, const HitRecord& rec, const Rayf& scattered) const
    {
        float cosine = dot(rec.normal, unit_vector(scattered.direction()));
        if (cosine < 0) return 0;
        return cosine / CUDART_PI_F;
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

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const
    {
        Vector3f reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        srec.specularRay = Rayf(rec.p, reflected + fuzz * randomInUnitSphere(rng));
        srec.attenuation = albedo;
        srec.isSpecular = true;
        //srec.pdf = nullptr;
        srec.cosinePdf = false;
        return true;
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

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const
    {
        srec.isSpecular = true;
        //srec.pdf = nullptr;
        srec.cosinePdf = false;
        srec.attenuation = Vector3f(1, 1, 1);
        Vector3f outwardNormal;
        Vector3f reflected = reflect(r_in.direction(), rec.normal);
        Vector3f refracted;
        float reflectProb, cosine;
        float niOverNt;
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
        if (rng.rand() < reflectProb)
        {
            srec.specularRay = Rayf(rec.p, reflected);
        }
        else
        {
            srec.specularRay = Rayf(rec.p, refracted);
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

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const
    {
        return false;
    }
    COMMON_FUNC virtual Vector3f emitted(const Rayf& r_in, const HitRecord& rec, const Vector2f& uv, const Vector3f& p) const
    {
        if (dot(rec.normal, r_in.direction()) < 0)
            return emit->value(uv, p);
        else
            return Vector3f(0, 0, 0);
    }

private:
    Texture* emit;
};

class Isotropic : public Material
{
public:
    COMMON_FUNC Isotropic(Texture* a) :
        albedo(a) {}

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const
    {
        // TODO: fix this for new ScatterRecord
        srec.isSpecular = false;
        srec.attenuation = albedo->value(rec.uv, rec.p);
        //srec.pdf = new ConstPdf();
        srec.cosinePdf = false;
        return true;
    }

    COMMON_FUNC virtual float scatteringPdf(const Rayf& r_in, const HitRecord& rec, const Rayf& scattered) const
    {
        return 1.0f / (4.0f * CUDART_PI_F);
    }

private:
    Texture* albedo;
};

#endif //PATHTRACER_MATERIAL_H