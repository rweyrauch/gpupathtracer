/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_SPHERE_H
#define PATHTRACER_SPHERE_H

#include "ptCudaCommon.h"
#include "ptHitable.h"
#include "ptPDF.h"
#include "ptONB.h"
#include "ptAABB.h"

COMMON_FUNC void get_uv(const Vector3f& p, Vector2f& uv)
{
    float phi = atan2f(p.z(), p.x());
    float theta = asinf(p.y());
    uv.u() = 1 - (phi + M_PI) / (2 * M_PI);
    uv.v() = (theta + M_PI/2) / M_PI;
}

class Sphere : public Hitable {
public:
    COMMON_FUNC Sphere() { }

    COMMON_FUNC Sphere(const Vector3f& cen, float r, Material* m) :
        center(cen),
        radius(r),
        material(m) { }

    COMMON_FUNC virtual bool hit(const Rayf& r, float tmin, float tmax, HitRecord& rec) const
    {
        Vector3f oc = r.origin() - center;
        float a = dot(r.direction(), r.direction());
        float b = dot(oc, r.direction());
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b*b-a*c;
        if (discriminant > 0)
        {
            float temp = (-b - Sqrt(discriminant)) / a;
            if (temp < tmax && temp > tmin)
            {
                rec.t = temp;
                rec.p = r.pointAt(rec.t);
                rec.normal = (rec.p-center)/radius;
                rec.material = material;
                return true;
            }
            temp = (-b + Sqrt(discriminant)) / a;
            if (temp<tmax && temp>tmin) {
                rec.t = temp;
                rec.p = r.pointAt(rec.t);
                rec.normal = (rec.p-center)/radius;
                rec.material = material;
                return true;
            }
        }
        return false;
    }

    Vector3f center;
    float radius;
    Material* material;
};

class MovingSphere : public Hitable
{
public:
    COMMON_FUNC MovingSphere() {}
    COMMON_FUNC MovingSphere(const Vector3f& cen0, const Vector3f& cen1, float t0, float t1, float r, Material* mtl) :
        center0(cen0),
        center1(cen1),
        time0(t0),
        time1(t1),
        radius(r),
        material(mtl)
    {}

    COMMON_FUNC virtual bool hit(const Rayf& ray, float t_min, float t_max, HitRecord& rec) const
    {
        Vector3f oc = ray.origin() - center(ray.time());
        float a = dot(ray.direction(), ray.direction());
        float b = dot(oc, ray.direction());
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - a * c;
        if (discriminant > 0)
        {
            float temp = (-b - Sqrt(discriminant)) / a;
            if (temp < t_max && temp > t_min)
            {
                rec.t = temp;
                rec.p = ray.pointAt(rec.t);
                rec.normal = (rec.p - center(ray.time())) / radius;
                rec.material = material;
                get_uv(rec.p, rec.uv);
                return true;
            }
            temp = (-b + Sqrt(discriminant)) / a;
            if (temp < t_max && temp > t_min)
            {
                rec.t = temp;
                rec.p = ray.pointAt(rec.t);
                rec.normal = (rec.p - center(ray.time())) / radius;
                rec.material = material;
                get_uv(rec.p, rec.uv);
                return true;
            }
        }
        return false;
    }

    COMMON_FUNC Vector3<float> center(float time) const
    {
        return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
    }

    Vector3f center0, center1;
    float time0, time1;
    float radius;
    Material* material;
};

#endif //PATHTRACER_SPHERE_H