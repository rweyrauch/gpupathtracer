/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */
#include "ptSphere.h"

bool Sphere::hit(const Rayf &r, float tmin, float tmax, HitRecord &rec, RNG &rng) const
{
    Vector3f oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;
    if (discriminant > 0)
    {
        float temp = (-b - Sqrt(discriminant)) / a;
        if (temp < tmax && temp > tmin)
        {
            rec.t = temp;
            rec.p = r.pointAt(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.material = material;
            return true;
        }
        temp = (-b + Sqrt(discriminant)) / a;
        if (temp < tmax && temp > tmin)
        {
            rec.t = temp;
            rec.p = r.pointAt(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.material = material;
            return true;
        }
    }
    return false;
}

bool Sphere::serialize(Stream *pStream) const
{
    if (pStream == nullptr)
        return false;

    const int id = typeId();
    bool ok = pStream->write(&id, sizeof(id));
    ok |= center.serialize(pStream);
    ok |= pStream->write(&radius, sizeof(radius));
    ok |= material->serialize(pStream);
    return ok;
}

bool Sphere::unserialize(Stream* pStream)
{
    if (pStream == nullptr)
        return false;

    bool ok = center.unserialize(pStream);
    ok |= pStream->read(&radius, sizeof(radius));
    material = Material::Create(pStream);
    return ok;
}

bool MovingSphere::hit(const Rayf &ray, float t_min, float t_max, HitRecord &rec, RNG &rng) const
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

bool MovingSphere::serialize(Stream *pStream) const
{
    if (pStream == nullptr)
        return false;

    const int id = typeId();
    bool ok = pStream->write(&id, sizeof(id));
    ok |= center0.serialize(pStream);
    ok |= center1.serialize(pStream);
    ok |= pStream->write(&time0, sizeof(time0));
    ok |= pStream->write(&time1, sizeof(time1));
    ok |= pStream->write(&radius, sizeof(radius));
    ok |= material->serialize(pStream);

    return ok;
}

bool MovingSphere::unserialize(Stream* pStream)
{
    if (pStream == nullptr)
        return false;

    bool ok = center0.unserialize(pStream);
    ok |= center1.unserialize(pStream);
    ok |= pStream->read(&time0, sizeof(time0));
    ok |= pStream->read(&time1, sizeof(time1));
    ok |= pStream->read(&radius, sizeof(radius));
    material = Material::Create(pStream);

    return ok;
}
