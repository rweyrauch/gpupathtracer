/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_HITABLE_H
#define PATHTRACER_HITABLE_H

#include "ptCudaCommon.h"
#include "ptVector3.h"
#include "ptVector2.h"
#include "ptRay.h"
#include "ptAABB.h"

class Material;
class RNG;
class Stream;

struct HitRecord
{
    float t;
    Vector3f p;
    Vector3f normal;
    Material* material;
    Vector2f uv;
};

enum HitableTypeId
{
  ListTypeId,
  SphereTypeId,
  MovingSphereTypeId,
  XYRectangleTypeId,
  XZRectangleTypeId,
  YZRectangleTypeId,
  FlipNormalsTypeId,
  BoxTypeId,
  TranslateTypeId,
  RotateYTypeId,
  MediumTypeId,
  BVHTypeId
};

class Hitable
{
public:
    COMMON_FUNC Hitable() = default;
    COMMON_FUNC virtual ~Hitable() = default;
    COMMON_FUNC virtual bool hit(const Rayf& r, float t_min, float t_max, HitRecord& rec, RNG& rng) const = 0;
    COMMON_FUNC virtual bool bounds(float t0, float t1, AABB<float>& bbox) const = 0;
    COMMON_FUNC virtual float pdfValue(const Vector3f& o, const Vector3f& v, RNG& rng) const { return 0; }
    COMMON_FUNC virtual Vector3f random(const Vector3f& o, RNG& rng) const { return Vector3f(1, 0, 0); }
    COMMON_FUNC virtual bool serialize(Stream* pStream) const = 0;
    COMMON_FUNC virtual bool deserialize(Stream *pStream) = 0;
    COMMON_FUNC virtual int typeId() const = 0;

    COMMON_FUNC static Hitable* Create(Stream* pStream);
};

#endif //PATHTRACER_HITABLE_H