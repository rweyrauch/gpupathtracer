/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_RECTANGLE_H
#define PATHTRACER_RECTANGLE_H

#include <math_constants.h>
#include <cfloat>
#include "ptCudaCommon.h"
#include "ptHitable.h"
#include "ptHitableList.h"
#include "ptAABB.h"
#include "ptRNG.h"
#include "ptMaterial.h"

const float RECT_TOLERANCE = 0.0001f;

class XYRectangle : public Hitable
{
public:
    COMMON_FUNC XYRectangle() = default;
    COMMON_FUNC XYRectangle(float X0, float X1, float Y0, float Y1, float K, Material* mat) :
        material(mat),
        x0(X0),
        x1(X1),
        y0(Y0),
        y1(Y1),
        k(K) {}

    COMMON_FUNC bool hit(const Rayf& r_in, float t0, float t1, HitRecord& rec) const override
    {
        float t = (k - r_in.origin().z()) / r_in.direction().z();
        if (t < t0 || t > t1) return false;
        float x = r_in.origin().x() + t * r_in.direction().x();
        float y = r_in.origin().y() + t * r_in.direction().y();
        if (x < x0 || x > x1 || y < y0 || y > y1) return false;

        rec.uv.u() = (x-x0)/(x1-x0);
        rec.uv.v() = (y-y0)/(y1-y0);
        rec.t = t;
        rec.material = material;
        rec.p = r_in.pointAt(t);
        rec.normal = Vector3f(0, 0, 1);

        return true;
    }

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override
    {
        bbox = AABB<float>(Vector3f(x0, y0, k-RECT_TOLERANCE), Vector3f(x1, y1, k+RECT_TOLERANCE));
        return true;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= material->serialize(pStream);
        ok |= pStream->write(&x0, sizeof(x0));
        ok |= pStream->write(&x1, sizeof(x1));
        ok |= pStream->write(&y0, sizeof(y0));
        ok |= pStream->write(&y1, sizeof(y1));
        ok |= pStream->write(&k, sizeof(k));

        return ok;
    }

    COMMON_FUNC int typeId() const override { return XYRectangleTypeId; }

private:
    Material* material;
    float x0, x1, y0, y1, k;
};

class XZRectangle : public Hitable
{
public:
    COMMON_FUNC XZRectangle() = default;
    COMMON_FUNC XZRectangle(float X0, float X1, float Z0, float Z1, float K, Material* mat) :
        material(mat),
        x0(X0),
        x1(X1),
        z0(Z0),
        z1(Z1),
        k(K) {}

    COMMON_FUNC bool hit(const Rayf& r_in, float t0, float t1, HitRecord& rec) const override
    {
        float t = (k - r_in.origin().y()) / r_in.direction().y();
        if (t < t0 || t > t1) return false;
        float x = r_in.origin().x() + t * r_in.direction().x();
        float z = r_in.origin().z() + t * r_in.direction().z();
        if (x < x0 || x > x1 || z < z0 || z > z1) return false;

        rec.uv.u() = (x-x0)/(x1-x0);
        rec.uv.v() = (z-z0)/(z1-z0);
        rec.t = t;
        rec.material = material;
        rec.p = r_in.pointAt(t);
        rec.normal = Vector3f(0, 1, 0);

        return true;
    }

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override
    {
        bbox = AABB<float>(Vector3f(x0, k-RECT_TOLERANCE, z0), Vector3f(x1, k+RECT_TOLERANCE, z1));
        return true;
    }

    COMMON_FUNC float pdfValue(const Vector3f& o, const Vector3f& v) const override
    {
        HitRecord rec;
        if (hit(Rayf(o, v), 0.001f, FLT_MAX, rec))
        {
            float area = (x1-x0) * (z1-z0);
            float distSqrd = rec.t * rec.t * v.squared_length();
            float cosine = fabsf(dot(v, rec.normal) / v.length());
            return distSqrd / (cosine * area);
        }
        else
            return 0;
    }

    COMMON_FUNC Vector3f random(const Vector3f& o, RNG& rng) const override
    {
        Vector3f randPoint = Vector3f(x0 + rng.rand() * (x1-x0), k, z0 + rng.rand() * (z1-z0));
        return randPoint - o;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= material->serialize(pStream);
        ok |= pStream->write(&x0, sizeof(x0));
        ok |= pStream->write(&x1, sizeof(x1));
        ok |= pStream->write(&z0, sizeof(z0));
        ok |= pStream->write(&z1, sizeof(z1));
        ok |= pStream->write(&k, sizeof(k));

        return ok;
    }

    COMMON_FUNC int typeId() const override { return XZRectangleTypeId; }

private:
    Material* material;
    float x0, x1, z0, z1, k;
};

class YZRectangle : public Hitable
{
public:
    COMMON_FUNC YZRectangle() = default;
    COMMON_FUNC YZRectangle(float Y0, float Y1, float Z0, float Z1, float K, Material* mat) :
        material(mat),
        y0(Y0),
        y1(Y1),
        z0(Z0),
        z1(Z1),
        k(K) {}

    COMMON_FUNC bool hit(const Rayf& r_in, float t0, float t1, HitRecord& rec) const override
    {
        float t = (k - r_in.origin().x()) / r_in.direction().x();
        if (t < t0 || t > t1) return false;
        float y = r_in.origin().y() + t * r_in.direction().y();
        float z = r_in.origin().z() + t * r_in.direction().z();
        if (y < y0 || y > y1 || z < z0 || z > z1) return false;

        rec.uv.u() = (y-y0)/(y1-y0);
        rec.uv.v() = (z-z0)/(z1-z0);
        rec.t = t;
        rec.material = material;
        rec.p = r_in.pointAt(t);
        rec.normal = Vector3f(1, 0, 0);

        return true;
    }

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override
    {
        bbox = AABB<float>(Vector3f(k-RECT_TOLERANCE, y0, z0), Vector3f(k+RECT_TOLERANCE, y1, z1));
        return true;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= material->serialize(pStream);
        ok |= pStream->write(&y0, sizeof(y0));
        ok |= pStream->write(&y1, sizeof(y1));
        ok |= pStream->write(&z0, sizeof(z0));
        ok |= pStream->write(&z1, sizeof(z1));
        ok |= pStream->write(&k, sizeof(k));

        return ok;
    }

    COMMON_FUNC int typeId() const override { return YZRectangleTypeId; }

private:
    Material* material;
    float y0, y1, z0, z1, k;
};

class FlipNormals : public Hitable
{
public:
    COMMON_FUNC explicit FlipNormals(Hitable* p) :
        hitable(p) {}

    COMMON_FUNC bool hit(const Rayf& r_in, float t0, float t1, HitRecord& rec) const override
    {
        if (hitable->hit(r_in, t0, t1, rec))
        {
            rec.normal = -rec.normal;
            return true;
        }
        return false;
    }

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override
    {
        return hitable->bounds(t0, t1, bbox);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= hitable->serialize(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return FlipNormalsTypeId; }

private:
    Hitable* hitable;
};

class Box : public Hitable
{
public:
    COMMON_FUNC Box() = default;
    COMMON_FUNC Box(const Vector3f& p0, const Vector3f& p1, Material* mat) :
        pmin(p0),
        pmax(p1)
    {
        int i = 0;
        Hitable** hl = new Hitable*[6];
        hl[i++] = new XYRectangle(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), mat);
        hl[i++] = new FlipNormals(new XYRectangle(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), mat));
        hl[i++] = new XZRectangle(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), mat);
        hl[i++] = new FlipNormals(new XZRectangle(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), mat));
        hl[i++] = new YZRectangle(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), mat);
        hl[i++] = new FlipNormals(new YZRectangle(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), mat));
        child = new HitableList(i, hl);
    }

    COMMON_FUNC bool hit(const Rayf& r_in, float t0, float t1, HitRecord& rec) const override
    {
        return child->hit(r_in, t0, t1, rec);
    }

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override
    {
        bbox = AABB<float>(pmin, pmax);
        return true;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= pmin.serialize(pStream);
        ok |= pmax.serialize(pStream);
        ok |= child->serialize(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return BoxTypeId; }

private:
    Vector3f pmin, pmax;
    Hitable* child;
};

class Translate : public Hitable
{
public:
    COMMON_FUNC Translate(Hitable *p, const Vector3f &displacement) :
        hitable(p),
        offset(displacement)
    {}

    COMMON_FUNC bool hit(const Rayf &r_in, float t0, float t1, HitRecord &rec) const override
    {
        Rayf movedR(r_in.origin() - offset, r_in.direction(), r_in.time());
        if (hitable->hit(movedR, t0, t1, rec))
        {
            rec.p += offset;
            return true;
        }
        return false;
    }

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float> &bbox) const override
    {
        if (hitable->bounds(t0, t1, bbox))
        {
            bbox = AABB<float>(bbox.min() + offset, bbox.max() + offset);
            return true;
        }
        return false;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= hitable->serialize(pStream);
        ok |= offset.serialize(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return TranslateTypeId; }

private:
    Hitable* hitable;
    Vector3f offset;
};

class RotateY : public Hitable
{
public:
    COMMON_FUNC RotateY(Hitable* p, float angle)
    {
        hitable = p;
        float radians = (CUDART_PI_F / 180) * angle;
        sinTheta = Sin(radians);
        cosTheta = Cos(radians);
        hasBox = p->bounds(0, 1, bbox);
        Vector3f min(FLT_MAX, FLT_MAX, FLT_MAX);
        Vector3f max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                for (int k = 0; k < 2; k++)
                {
                    float x = i * bbox.max().x() + (1-i)*bbox.min().x();
                    float y = j * bbox.max().y() + (1-j)*bbox.min().y();
                    float z = k * bbox.max().z() + (1-k)*bbox.min().z();
                    float newx = cosTheta * x + sinTheta * z;
                    float newz = -sinTheta * x + cosTheta * z;
                    Vector3f tester(newx, y, newz);
                    for (int c = 0; c < 3; c++)
                    {
                        if (tester[c] > max[c])
                            max[c] = tester[c];
                        if (tester[c] < min[c])
                            min[c] = tester[c];
                    }
                }
            }
        }
        bbox = AABB<float>(min, max);
    }

    COMMON_FUNC bool hit(const Rayf& r_in, float t0, float t1, HitRecord& rec) const override
    {
        auto origin = r_in.origin();
        auto direction = r_in.direction();

        origin[0] = cosTheta*r_in.origin()[0] - sinTheta*r_in.origin()[2];
        origin[2] = sinTheta*r_in.origin()[0] + cosTheta*r_in.origin()[2];

        direction[0] = cosTheta*r_in.direction()[0] - sinTheta*r_in.direction()[2];
        direction[2] = sinTheta*r_in.direction()[0] + cosTheta*r_in.direction()[2];

        Rayf rotatedR(origin, direction, r_in.time());
        if (hitable->hit(rotatedR, t0, t1, rec))
        {
            Vector3f p = rec.p;
            Vector3f normal = rec.normal;
            p[0] = cosTheta*rec.p[0] + sinTheta*rec.p[2];
            p[2] = -sinTheta*rec.p[0] + cosTheta*rec.p[2];
            normal[0] = cosTheta*rec.normal[0] + sinTheta*rec.normal[2];
            normal[2] = -sinTheta*rec.normal[0] + cosTheta*rec.normal[2];
            rec.p = p;
            rec.normal = normal;
            return true;
        }
        return false;
    }

    COMMON_FUNC bool bounds(float t0, float t1, AABB<float>& bbox) const override
    {
        bbox = this->bbox;
        return hasBox;
    }

    COMMON_FUNC bool serialize(Stream* pStream) const override
    {
        if (pStream == nullptr)
            return false;

        const int id = typeId();
        bool ok = pStream->write(&id, sizeof(id));
        ok |= hitable->serialize(pStream);
        ok |= pStream->write(&sinTheta, sizeof(sinTheta));
        ok |= pStream->write(&cosTheta, sizeof(cosTheta));
        ok |= pStream->write(&hasBox, sizeof(hasBox));
        ok |= bbox.serialize(pStream);

        return ok;
    }

    COMMON_FUNC int typeId() const override { return RotateYTypeId; }

private:
    Hitable* hitable;
    float sinTheta, cosTheta;
    bool hasBox;
    AABB<float> bbox;
};

#endif //PATHTRACER_RECTANGLE_H
