/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include <math_constants.h>
#include "ptCamera.h"

Camera::Camera(float vfov, float aspect) :
    origin(0, 0, 0.),
    lowerLeftCorner(),
    horizontal(),
    vertical()
{
    float theta = vfov * CUDART_PI_F / 180;
    float halfHeight = Tan(theta / 2);
    float halfWidth = aspect * halfHeight;
    lowerLeftCorner = Vector3f(-halfWidth, -halfHeight, -1);
    horizontal = Vector3f(2 * halfWidth, 0, 0);
    vertical = Vector3f(0, 2 * halfHeight, 0);
}

Camera::Camera(const Vector3f& from, const Vector3f& to, const Vector3f& vup, float vfov, float aspect, float aperture, float focal_dist, float t0, float t1) :
    origin(from),
    lowerLeftCorner(),
    horizontal(),
    vertical(),
    time0(t0),
    time1(t1)
{
    lens_radius = aperture / 2;
    float theta = vfov * CUDART_PI_F / 180;
    float halfHeight = Tan(theta / 2);
    float halfWidth = aspect * halfHeight;
    w = unit_vector(from - to);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);
    lowerLeftCorner = origin - halfWidth * focal_dist * u - halfHeight * focal_dist * v - focal_dist * w;
    horizontal = 2 * halfWidth * focal_dist * u;
    vertical = 2 * halfHeight * focal_dist * v;
}

bool Camera::serialize(Stream* pStream) const
{
    if (pStream == nullptr)
        return false;

    bool ok = origin.serialize(pStream);
    ok |= lowerLeftCorner.serialize(pStream);
    ok |= horizontal.serialize(pStream);
    ok |= vertical.serialize(pStream);
    ok |= u.serialize(pStream);
    ok |= v.serialize(pStream);
    ok |= w.serialize(pStream);
    ok |= pStream->write(&time0, sizeof(time0));
    ok |= pStream->write(&time1, sizeof(time1));
    ok |= pStream->write(&lens_radius, sizeof(lens_radius));

    return ok;
}

bool Camera::deserialize(Stream *pStream)
{
    if (pStream == nullptr)
        return false;

    bool ok = origin.deserialize(pStream);
    ok |= lowerLeftCorner.deserialize(pStream);
    ok |= horizontal.deserialize(pStream);
    ok |= vertical.deserialize(pStream);
    ok |= u.deserialize(pStream);
    ok |= v.deserialize(pStream);
    ok |= w.deserialize(pStream);
    ok |= pStream->read(&time0, sizeof(time0));
    ok |= pStream->read(&time1, sizeof(time1));
    ok |= pStream->read(&lens_radius, sizeof(lens_radius));

    return ok;
}

Camera* Camera::Create(Stream* pStream)
{
    if (pStream == nullptr )
        return nullptr;

    Camera* cam = new Camera;
    bool ok = cam->deserialize(pStream);
    if (!ok)
    {
        delete cam;
        cam = nullptr;
    }
    return cam;
}
