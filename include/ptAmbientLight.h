/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_AMBIENTLIGHT_H
#define PATHTRACER_AMBIENTLIGHT_H

#include "ptCudaCommon.h"
#include "ptVector3.h"
#include "ptRay.h"
#include "ptStream.h"

enum AmbientLightTypeId
{
  ConstantAmbientTypeId,
  SkyAmbientTypeId,
};

class AmbientLight
{
public:
    COMMON_FUNC virtual ~AmbientLight() = default;

    COMMON_FUNC virtual Vector3f emitted(const Rayf& ray) const = 0;

    COMMON_FUNC virtual bool serialize(Stream* pStream) const = 0;
    COMMON_FUNC virtual bool deserialize(Stream *pStream) = 0;

    COMMON_FUNC virtual int typeId() const = 0;

    COMMON_FUNC static AmbientLight* Create(Stream* pStream);
};

class ConstantAmbient : public AmbientLight
{
public:
    COMMON_FUNC ConstantAmbient() :
        color(0, 0, 0) {}

    COMMON_FUNC explicit ConstantAmbient(const Vector3f& c) :
        color(c) {}

    COMMON_FUNC Vector3f emitted(const Rayf& ray) const override
    {
        return color;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= color.serialize(pStream);

        return ok;
    }

    COMMON_FUNC bool deserialize(Stream *pStream) override
    {
        if (pStream == nullptr)
            return false;

        bool ok = color.serialize(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return ConstantAmbientTypeId; }

private:
    Vector3f color;
};

class SkyAmbient : public AmbientLight
{
public:
    COMMON_FUNC SkyAmbient() = default;

    COMMON_FUNC Vector3f emitted(const Rayf& ray) const override
    {
        Vector3f unit_dir = unit_vector(ray.direction());
        float t = 0.5f * (unit_dir.y() + 1);
        return (1 - t) * Vector3f(1, 1, 1) + t * Vector3f(0.5f, 0.7f, 1.0f);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));

        return ok;
    }

    COMMON_FUNC bool deserialize(Stream *pStream) override
    {
        if (pStream == nullptr)
            return false;

        return true;
    }

    COMMON_FUNC int typeId() const override { return SkyAmbientTypeId; }
};

#endif //PATHTRACER_AMBIENTLIGHT_H
