/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptTriangle.h"
#include "ptMaterial.h"

Triangle::Triangle(const Vector3f& v0, const Vector2f& t0,
         const Vector3f& v1, const Vector2f& t1,
         const Vector3f& v2, const Vector2f& t2,
         Material* mtl) :
    v0(v0),
    v1(v1),
    v2(v2),
    t0(t0),
    t1(t1),
    t2(t2),
    material(mtl)
    {
        calcBounds();
    }

bool Triangle::hit(const Rayf& r, float t_min, float t_max, HitRecord& rec, RNG& rng) const
{
    //
    // Tomas Moller and Ben Trumbore, "Fast Minimum Storage Ray-Triangle Intersection,"
    // Journal of Graphics Tools, Vol. 2, No. 1, pp. 21-28, 1997.
    //

    // Find vectors for two edges sharing v0.
    Vector3f edge1(v1 - v0);
    Vector3f edge2(v2 - v0);

    // Begin calculating determinant - also used to calculate U parameter.
    Vector3f pvec = cross(r.direction(), edge2);

    // If determinant is near zero, ray lies in plane of triangle.
    auto det = dot(edge1, pvec);

    if (det < 0.0001f)
        return false;

    // Calculate distance from v0 to ray origin.
    Vector3f tvec(r.origin() - v0);

    // calculate U parameter and test bounds.
    float u = dot(tvec, pvec);
    if (u < 0 || u > det)
        return false;

    // Prepare to test V parameter.
    Vector3f qvec = cross(tvec, edge1);

    // Calculate V parameter and test bounds.
    auto v = dot(r.direction(), qvec);
    if (v < 0 || u + v > det)
        return false;

    // Calculate t, scale parameters, ray intersects triangle.
    float t = dot(edge2, qvec);
    if (t < t_min || t > t_max) return false;

    const auto inv_det = 1 / det;
    u *= inv_det;
    v *= inv_det;

    rec.t = t;
    rec.p = (1 - u - v) * v0 + u * v1 + v * v2;
    rec.normal = cross(edge1, edge2);
    rec.normal.make_unit_vector();
    rec.material = material;

    Vector3f bary(1.0 - u - v, u, v);
    calcTexCoord(bary, rec.uv);

    return true;
}

bool Triangle::bounds(float t0, float t1, AABB<float>& bbox) const
{
    bbox = this->bbox;
    return true;
}

float Triangle::area() const
{
    Vector3f u(v1 - v0);
    Vector3f v(v2 - v0);
    Vector3f uv = cross(u, v);
    return 0.5f * uv.length();
}

void Triangle::calcTexCoord(const Vector3f& bary, Vector2f& uv) const
{
    uv = t0 * bary.x() + t1 * bary.y() + t2 * bary.z();
}

void Triangle::calcBounds()
{
    Vector3f bmin{}, bmax{};
    for (int i = 0; i < 3; i++)
    {
        bmin[i] = Min(v0[i]-0.0001f, Min(v1[i]-0.0001f, v2[i]-0.0001f));
        bmax[i] = Max(v0[i]+0.0001f, Max(v1[i]+0.0001f, v2[i]+0.0001f));
    }
    bbox = AABB<float>(bmin, bmax);
}

bool Triangle::serialize(Stream *pStream) const
{
    if (pStream == nullptr)
        return false;

    const int id = typeId();
    bool ok = pStream->write(&id, sizeof(id));
    ok |= v0.serialize(pStream);
    ok |= v1.serialize(pStream);
    ok |= v2.serialize(pStream);
    ok |= t0.serialize(pStream);
    ok |= t1.serialize(pStream);
    ok |= t2.serialize(pStream);
    ok |= material->serialize(pStream);
    ok |= bbox.serialize(pStream);

    return ok;
}

bool Triangle::deserialize(Stream *pStream)
{
    if (pStream == nullptr)
        return false;

    bool ok = v0.deserialize(pStream);
    ok |= v1.deserialize(pStream);
    ok |= v2.deserialize(pStream);
    ok |= t0.deserialize(pStream);
    ok |= t1.deserialize(pStream);
    ok |= t2.deserialize(pStream);
    material = Material::Create(pStream);
    ok |= bbox.deserialize(pStream);

    return ok;
}


bool TriangleMesh::hit(const Rayf& r, float t_min, float t_max, HitRecord& rec, RNG& rng) const
{
    return false;
}

bool TriangleMesh::hit(const Rayf& ray, const TriangleFast& accel, float& tHit, Vector3f& bary) const
{
    //
    // "Real Time Ray Tracing and Interactive Global Illumination", Ingo Wald:
    // http://www.mpi-sb.mpg.de/~wald/PhD/
    //
    // Jakko Bikker
    // http://www.flipcode.com/articles/article_raytrace07.shtml
    //
    static int axisModulo[] = { 0, 1, 2, 0, 1 };
    const int ku = axisModulo[accel.m_k+1];
    const int kv = axisModulo[accel.m_k+2];

    const float nd = 1 / (ray.direction()[accel.m_k] + accel.m_nu * ray.direction()[ku] + accel.m_nv * ray.direction()[kv]);
    float t = (accel.m_nd - ray.origin()[accel.m_k] - accel.m_nu * ray.origin()[ku] - accel.m_nv * ray.origin()[kv]) * nd;

    if (t < 0)
    {
        return false;
    }

    const float hu = ray.origin()[ku] + t * ray.direction()[ku] - accel.m_v0[ku];
    const float hv = ray.origin()[kv] + t * ray.direction()[kv] - accel.m_v0[kv];

    const float u = hv * accel.m_bnu + hu * accel.m_bnv;
    if (u < 0)
        return false;

    const float v = hu * accel.m_cnu + hv * accel.m_cnv;
    if (v < 0)
        return false;

    if (u + v > 1)
        return false;

    tHit = t;
    bary = Vector3f(1 - u - v, u, v);

    return true;
}

bool TriangleMesh::bounds(float t0, float t1, AABB<float>& bbox) const
{
    return false;
}

void TriangleMesh::addVertex(const Vector3f& p, const Vector3f& n, const Vector2f& tex)
{
    verts.push_back(p);
    normals.push_back(n);
    texCoords.push_back(tex);
}

void TriangleMesh::complete()
{
    delete[] triAccel;
    count = triangles.size();
    triAccel = new TriangleFast[count];

    int i = 0;
    for (const auto ip : triangles)
    {
        TriangleFast triFast(verts[ip.i0], verts[ip.i1],  verts[ip.i2]);
        triAccel[i++] = triFast;
    }
}

bool TriangleMesh::serialize(Stream *pStream) const
{
    if (pStream == nullptr)
        return false;

    const int id = typeId();
    bool ok = pStream->write(&id, sizeof(id));
    ok |= pStream->write(&count, sizeof(count));
    for (int i = 0; i < count && ok; i++)
    {
        ok |= triAccel[i].m_v0.serialize(pStream);
        ok |= pStream->write(&triAccel[i].m_nu, sizeof(float));
        ok |= pStream->write(&triAccel[i].m_nv, sizeof(float));
        ok |= pStream->write(&triAccel[i].m_nd, sizeof(float));
        ok |= pStream->write(&triAccel[i].m_k, sizeof(int));
        ok |= pStream->write(&triAccel[i].m_bnu, sizeof(float));
        ok |= pStream->write(&triAccel[i].m_bnv, sizeof(float));
        ok |= pStream->write(&triAccel[i].m_cnu, sizeof(float));
        ok |= pStream->write(&triAccel[i].m_cnv, sizeof(float));
    }

    return ok;
}

bool TriangleMesh::deserialize(Stream *pStream)
{
    if (pStream == nullptr)
        return false;

    bool ok = pStream->read(&count, sizeof(count));
    if (ok && (count > 0))
    {
        triAccel = new TriangleFast[count];
        for (int i = 0; i < count && ok; i++)
        {
            ok |= triAccel[i].m_v0.deserialize(pStream);
            ok |= pStream->read(&triAccel[i].m_nu, sizeof(float));
            ok |= pStream->read(&triAccel[i].m_nv, sizeof(float));
            ok |= pStream->read(&triAccel[i].m_nd, sizeof(float));
            ok |= pStream->read(&triAccel[i].m_k, sizeof(int));
            ok |= pStream->read(&triAccel[i].m_bnu, sizeof(float));
            ok |= pStream->read(&triAccel[i].m_bnv, sizeof(float));
            ok |= pStream->read(&triAccel[i].m_cnu, sizeof(float));
            ok |= pStream->read(&triAccel[i].m_cnv, sizeof(float));
        }
    }

    return ok;
}

TriangleMesh::TriangleFast::TriangleFast(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2)
{
    m_v0 = v0;

    // Find vectors for two edges sharing v0.
    Vector3f c(v1 - v0);
    Vector3f b(v2 - v0);

    // Compute normal
    Vector3f N = cross(b, c);

    // Identify primary plane
    if (fabs(N.x()) > fabs(N.y()))
    {
        if (fabs(N.x()) > fabs(N.z()))
            m_k = 0;
        else
            m_k = 2;
    }
    else
    {
        if (fabs(N.y()) > fabs(N.z()))
            m_k = 1;
        else
            m_k = 2;
    }

    // Compute triangle plane coefficients in projection plane
    int u = (m_k+1) % 3;
    int v = (m_k+2) % 3;
    float invNormPP = 1.0f / N[m_k];

    m_nu = N[u] * invNormPP;
    m_nv = N[v] * invNormPP;
    m_nd = dot(N, v0) * invNormPP;

    // Compute projection plane edge equations
    float invDet = 1.0f / (b[u] * c[v] - b[v] * c[u]);
    m_bnu = b[u] * invDet;
    m_bnv = -b[v] * invDet;

    m_cnu = c[v] * invDet;
    m_cnv = -c[u] * invDet;
}
