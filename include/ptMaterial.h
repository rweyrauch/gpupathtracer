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

enum MaterialTypeId
{
  LambertianTypeId = MakeFourCC('L','A','M','B'),
  MetalTypeId = MakeFourCC('M','E','T','L'),
  DielectricTypeId = MakeFourCC('D','I','E','L'),
  DiffuseLightTypeId = MakeFourCC('D','I','F','F'),
  IsotropicTypeId = MakeFourCC('I','S','O','T')
};

class Material
{
public:
    COMMON_FUNC virtual ~Material() {}

    COMMON_FUNC virtual bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const = 0;
    COMMON_FUNC virtual float scatteringPdf(const Rayf& r_in, const HitRecord& rec, const Rayf& scattered) const { return 0; }
    COMMON_FUNC virtual Vector3f emitted(const Rayf& r_in, const HitRecord& rec, const Vector2f& uv, const Vector3f& p) const { return Vector3f(0, 0, 0); }
    COMMON_FUNC virtual bool serialize(Stream* pStream) const = 0;
    COMMON_FUNC virtual bool deserialize(Stream *pStream) = 0;
    COMMON_FUNC virtual int typeId() const = 0;

    COMMON_FUNC static Material* Create(Stream* pStream);
};

class Lambertian : public Material
{
public:
    COMMON_FUNC Lambertian() = default;

    COMMON_FUNC explicit Lambertian(Texture* a) :
        albedo(a) { }

    COMMON_FUNC bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const override
    {
        srec.isSpecular = false;
        srec.attenuation = albedo->value(rec.uv, rec.p);
        //srec.pdf = new CosinePdf(rec.normal);
        srec.cosinePdf = true;
        return true;
    }

    COMMON_FUNC float scatteringPdf(const Rayf& r_in, const HitRecord& rec, const Rayf& scattered) const override
    {
        float cosine = dot(rec.normal, unit_vector(scattered.direction()));
        if (cosine < 0) return 0;
        return cosine / CUDART_PI_F;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        if (albedo != nullptr)
        {
            ok |= albedo->serialize(pStream);
        }
        else
        {
            const int nullId = -1;
            ok |= pStream->write(&nullId, sizeof(nullId));
        }

        return ok;
    }

    COMMON_FUNC bool deserialize(Stream *pStream) override
    {
        if (pStream == nullptr)
            return false;

        albedo = Texture::Create(pStream);

        return true;
    }

    COMMON_FUNC int typeId() const override { return LambertianTypeId; }

private:
    Texture* albedo;
};

class Metal : public Material
{
public:
    COMMON_FUNC Metal() = default;

    COMMON_FUNC Metal(const Vector3f& a, float f) :
        albedo(a),
        fuzz(1)
    {
        if (f < 1) { fuzz = f; }
    }

    COMMON_FUNC bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const override
    {
        Vector3f reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        srec.specularRay = Rayf(rec.p, reflected + fuzz * randomInUnitSphere(rng));
        srec.attenuation = albedo;
        srec.isSpecular = true;
        //srec.pdf = nullptr;
        srec.cosinePdf = false;
        return true;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= albedo.serialize(pStream);
        ok |= pStream->write(&fuzz, sizeof(fuzz));

        return ok;
    }

    COMMON_FUNC bool deserialize(Stream *pStream) override
    {
        if (pStream == nullptr)
            return false;

        bool ok = albedo.deserialize(pStream);
        ok |= pStream->read(&fuzz, sizeof(fuzz));

        return ok;
    }

    COMMON_FUNC int typeId() const override { return MetalTypeId; }

private:
    Vector3f albedo;
    float fuzz;
};

class Dielectric : public Material
{
public:
    COMMON_FUNC Dielectric() = default;

    COMMON_FUNC explicit Dielectric(float ri) :
        refIndex(ri) { }

    COMMON_FUNC bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const override
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

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= pStream->write(&refIndex, sizeof(refIndex));

        return ok;
    }

    COMMON_FUNC bool deserialize(Stream *pStream) override
    {
        if (pStream == nullptr)
            return false;

        bool ok = pStream->read(&refIndex, sizeof(refIndex));

        return ok;
    }

    COMMON_FUNC int typeId() const override { return DielectricTypeId; }

private:
    float refIndex;
};

class DiffuseLight : public Material
{
public:
    COMMON_FUNC DiffuseLight() = default;

    COMMON_FUNC explicit DiffuseLight(Texture* a) :
        emit(a) {}

    COMMON_FUNC bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const override
    {
        return false;
    }
    COMMON_FUNC Vector3f emitted(const Rayf& r_in, const HitRecord& rec, const Vector2f& uv, const Vector3f& p) const override
    {
        if (dot(rec.normal, r_in.direction()) < 0)
            return emit->value(uv, p);
        else
            return Vector3f(0, 0, 0);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        if (emit != nullptr)
        {
            ok |= emit->serialize(pStream);
        }
        else
        {
            const int nullId = -1;
            pStream->write(&nullId, sizeof(nullId));
        }

        return ok;
    }

    COMMON_FUNC bool deserialize(Stream *pStream) override
    {
        if (pStream == nullptr)
            return false;

        emit = Texture::Create(pStream);

        return true;
    }

    COMMON_FUNC int typeId() const override { return DiffuseLightTypeId; }

private:
    Texture* emit;
};

class Isotropic : public Material
{
public:
    COMMON_FUNC Isotropic() = default;

    COMMON_FUNC explicit Isotropic(Texture* a) :
        albedo(a) {}

    COMMON_FUNC bool scatter(const Rayf& r_in, const HitRecord& rec, ScatterRecord& srec, RNG& rng) const override
    {
        // TODO: fix this for new ScatterRecord
        srec.isSpecular = false;
        srec.attenuation = albedo->value(rec.uv, rec.p);
        //srec.pdf = new ConstPdf();
        srec.cosinePdf = false;
        return true;
    }

    COMMON_FUNC float scatteringPdf(const Rayf& r_in, const HitRecord& rec, const Rayf& scattered) const override
    {
        return 1.0f / (4.0f * CUDART_PI_F);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        if (albedo != nullptr)
        {
            ok |= albedo->serialize(pStream);
        }
        else
        {
            const int nullId = -1;
            pStream->write(&nullId, sizeof(nullId));
        }

        return ok;
    }

    COMMON_FUNC bool deserialize(Stream *pStream) override
    {
        if (pStream == nullptr)
            return false;

        albedo = Texture::Create(pStream);

        return true;
    }

    COMMON_FUNC int typeId() const override { return IsotropicTypeId; }

private:
    Texture* albedo;
};

#endif //PATHTRACER_MATERIAL_H