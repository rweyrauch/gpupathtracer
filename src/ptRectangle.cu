/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptRectangle.h"

bool XYRectangle::hit(const Rayf &r_in, float t0, float t1, HitRecord &rec, RNG &rng) const
{
    float t = (k - r_in.origin().z()) / r_in.direction().z();
    if (t < t0 || t > t1) return false;
    float x = r_in.origin().x() + t * r_in.direction().x();
    float y = r_in.origin().y() + t * r_in.direction().y();
    if (x < x0 || x > x1 || y < y0 || y > y1) return false;

    rec.uv.u() = (x - x0) / (x1 - x0);
    rec.uv.v() = (y - y0) / (y1 - y0);
    rec.t = t;
    rec.material = material;
    rec.p = r_in.pointAt(t);
    rec.normal = Vector3f(0, 0, 1);

    return true;
}

bool XYRectangle::serialize(Stream *pStream) const
{
    if (pStream == nullptr)
        return false;

    const int id = typeId();
    bool ok = pStream->write(&id, sizeof(id));
    if (material != nullptr)
        ok |= material->serialize(pStream);
    else
        ok |= pStream->writeNull();
    ok |= pStream->write(&x0, sizeof(x0));
    ok |= pStream->write(&x1, sizeof(x1));
    ok |= pStream->write(&y0, sizeof(y0));
    ok |= pStream->write(&y1, sizeof(y1));
    ok |= pStream->write(&k, sizeof(k));

    return ok;
}

bool XYRectangle::deserialize(Stream *pStream)
{
    if (pStream == nullptr)
        return false;

    material = Material::Create(pStream);
    bool ok = pStream->read(&x0, sizeof(x0));
    ok |= pStream->read(&x1, sizeof(x1));
    ok |= pStream->read(&y0, sizeof(y0));
    ok |= pStream->read(&y1, sizeof(y1));
    ok |= pStream->read(&k, sizeof(k));

    return ok;
}


bool XZRectangle::hit(const Rayf &r_in, float t0, float t1, HitRecord &rec, RNG &rng) const
{
    float t = (k - r_in.origin().y()) / r_in.direction().y();
    if (t < t0 || t > t1) return false;
    float x = r_in.origin().x() + t * r_in.direction().x();
    float z = r_in.origin().z() + t * r_in.direction().z();
    if (x < x0 || x > x1 || z < z0 || z > z1) return false;

    rec.uv.u() = (x - x0) / (x1 - x0);
    rec.uv.v() = (z - z0) / (z1 - z0);
    rec.t = t;
    rec.material = material;
    rec.p = r_in.pointAt(t);
    rec.normal = Vector3f(0, 1, 0);

    return true;
}

bool XZRectangle::serialize(Stream *pStream) const
{
    if (pStream == nullptr)
        return false;

    const int id = typeId();
    bool ok = pStream->write(&id, sizeof(id));
    if (material != nullptr)
        ok |= material->serialize(pStream);
    else
        ok |= pStream->writeNull();
    ok |= pStream->write(&x0, sizeof(x0));
    ok |= pStream->write(&x1, sizeof(x1));
    ok |= pStream->write(&z0, sizeof(z0));
    ok |= pStream->write(&z1, sizeof(z1));
    ok |= pStream->write(&k, sizeof(k));

    return ok;
}

bool XZRectangle::deserialize(Stream *pStream)
{
    if (pStream == nullptr)
        return false;

    material = Material::Create(pStream);
    bool ok = pStream->read(&x0, sizeof(x0));
    ok |= pStream->read(&x1, sizeof(x1));
    ok |= pStream->read(&z0, sizeof(z0));
    ok |= pStream->read(&z1, sizeof(z1));
    ok |= pStream->read(&k, sizeof(k));

    return ok;
}


bool YZRectangle::hit(const Rayf &r_in, float t0, float t1, HitRecord &rec, RNG &rng) const
{
    float t = (k - r_in.origin().x()) / r_in.direction().x();
    if (t < t0 || t > t1) return false;
    float y = r_in.origin().y() + t * r_in.direction().y();
    float z = r_in.origin().z() + t * r_in.direction().z();
    if (y < y0 || y > y1 || z < z0 || z > z1) return false;

    rec.uv.u() = (y - y0) / (y1 - y0);
    rec.uv.v() = (z - z0) / (z1 - z0);
    rec.t = t;
    rec.material = material;
    rec.p = r_in.pointAt(t);
    rec.normal = Vector3f(1, 0, 0);

    return true;
}

bool YZRectangle::serialize(Stream *pStream) const
{
    if (pStream == nullptr)
        return false;

    const int id = typeId();
    bool ok = pStream->write(&id, sizeof(id));
    if (material != nullptr)
        ok |= material->serialize(pStream);
    else
        ok |= pStream->writeNull();
    ok |= pStream->write(&y0, sizeof(y0));
    ok |= pStream->write(&y1, sizeof(y1));
    ok |= pStream->write(&z0, sizeof(z0));
    ok |= pStream->write(&z1, sizeof(z1));
    ok |= pStream->write(&k, sizeof(k));

    return ok;
}

bool YZRectangle::deserialize(Stream *pStream)
{
    if (pStream == nullptr)
        return false;

    material = Material::Create(pStream);
    bool ok = pStream->read(&y0, sizeof(y0));
    ok |= pStream->read(&y1, sizeof(y1));
    ok |= pStream->read(&z0, sizeof(z0));
    ok |= pStream->read(&z1, sizeof(z1));
    ok |= pStream->read(&k, sizeof(k));

    return ok;
}
