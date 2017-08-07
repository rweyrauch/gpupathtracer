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
  NullTypeId = -1,
  ListTypeId = 1, //MakeFourCC('L','I','S','T'),
  SphereTypeId, // = MakeFourCC('S','P', 'H', 'E'),
  MovingSphereTypeId, // = MakeFourCC('M','S','P','H'),
  XYRectangleTypeId, // = MakeFourCC('X','Y','R','E'),
  XZRectangleTypeId, // = MakeFourCC('X','Z','R','E'),
  YZRectangleTypeId, // = MakeFourCC('Y','Z','R','E'),
  FlipNormalsTypeId, // = MakeFourCC('F','L','I','P'),
  BoxTypeId, //= MakeFourCC('B','O','X','H'),
  TranslateTypeId, // = MakeFourCC('T','R','A','N'),
  RotateYTypeId, // = MakeFourCC('R','O','T','Y'),
  MediumTypeId, // = MakeFourCC('C','M','E','D'),
  BVHTypeId, // = MakeFourCC('B','V','H',' '),
  TriangleTypeId, // = MakeFourCC('T','R','I',' '),
  TriMeshTypeId // = MakeFourCC('M','E','S','H')
};

class Hitable
{
public:
    COMMON_FUNC Hitable() {}
    COMMON_FUNC virtual ~Hitable() {}
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