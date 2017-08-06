/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_TRIANGLE_H
#define PATHTRACER_TRIANGLE_H

#include <vector>
#include "ptHitable.h"
#include "ptTexture.h"
#include "ptVector2.h"
#include "ptAABB.h"

class Triangle : public Hitable
{
public:
    COMMON_FUNC Triangle(const Vector3f& v0, const Vector2f& t0,
             const Vector3f& v1, const Vector2f& t1,
             const Vector3f& v2, const Vector2f& t2,
             Material* mtl);

    COMMON_FUNC bool hit(const Rayf& r, float t_min, float t_max, HitRecord& rec, RNG& rng) const override;

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override;

    COMMON_FUNC bool serialize(Stream* pStream) const override;
    COMMON_FUNC bool deserialize(Stream *pStream) override;
    COMMON_FUNC int typeId() const override { return TriangleTypeId; }

    COMMON_FUNC float area() const;

private:

    COMMON_FUNC void calcTexCoord(const Vector3f& xyz, Vector2f& uv) const;
    COMMON_FUNC void calcBounds();

    Vector3f v0;
    Vector3f v1;
    Vector3f v2;

    Vector2f t0;
    Vector2f t1;
    Vector2f t2;

    Material* material;

    AABB<float> bbox;
};

struct TriIndex
{
    unsigned int i0, i1, i2;
};

class TriangleMesh : public Hitable
{
public:
    COMMON_FUNC explicit TriangleMesh(Material* mtl) :
        material(mtl) { }

    COMMON_FUNC bool hit(const Rayf& r, float t_min, float t_max, HitRecord& rec, RNG& rng) const override;

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override;

    COMMON_FUNC bool serialize(Stream* pStream) const override;
    COMMON_FUNC bool deserialize(Stream *pStream) override;
    COMMON_FUNC int typeId() const override { return TriMeshTypeId; }

    void addVertex(const Vector3f& p, const Vector3f& n, const Vector2f& tex);

    void addTriangle(const TriIndex& tri)
    {
        triangles.push_back(tri);
    }

    void complete();

private:

    struct TriangleFast
    {
        COMMON_FUNC TriangleFast() {}
        COMMON_FUNC TriangleFast(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2);

        Vector3f m_v0;
        float  m_nu, m_nv, m_nd;   // Plane coeff of projection plane
        int     m_k;                // Projection plane index (X, Y, Z)

        float  m_bnu, m_bnv;       // Projection plane edge equations
        float  m_cnu, m_cnv;
    };

    COMMON_FUNC bool hit(const Rayf& r, const TriangleFast& accel, float& t, Vector3f& bary) const;

    std::vector<Vector3f> verts;
    std::vector<Vector3f> normals;
    std::vector<Vector2f> texCoords;
    std::vector<TriIndex> triangles;

    int count = 0;
    TriangleFast* triAccel = nullptr;

    Material* material = nullptr;

    AABB<float> bbox;
};

#endif //PATHTRACER_TRIANGLE_H
