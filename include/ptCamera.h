/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef PATHTRACER_CAMERA_H
#define PATHTRACER_CAMERA_H

#include <math_constants.h>
#include "ptCudaCommon.h"
#include "ptMaterial.h"
#include "ptVector3.h"
#include "ptRay.h"
#include "ptRNG.h"

class Camera {
public:
    COMMON_FUNC Camera() = default;

    COMMON_FUNC Camera(float vfov, float aspect) :
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

    COMMON_FUNC Camera(const Vector3f& from, const Vector3f& to, const Vector3f& vup, float vfov, float aspect, float aperture, float focal_dist, float t0 = 0, float t1 = 1) :
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

    COMMON_FUNC Rayf getRay(float s, float t, RNG& rng)
    {
        Vector3f rd = lens_radius * randomInUnitDisk(rng);
        Vector3f offset = u * rd.x() + v * rd.y();
        float time = time0 + rng.rand() * (time1 - time0);
        return Rayf(origin + offset, lowerLeftCorner + s * horizontal + t * vertical - origin - offset, time);
    }

    COMMON_FUNC bool serialize(Stream* pStream) const
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

    COMMON_FUNC bool unserialize(Stream* pStream)
    {
        if (pStream == nullptr)
            return false;

        bool ok = origin.unserialize(pStream);
        ok |= lowerLeftCorner.unserialize(pStream);
        ok |= horizontal.unserialize(pStream);
        ok |= vertical.unserialize(pStream);
        ok |= u.unserialize(pStream);
        ok |= v.unserialize(pStream);
        ok |= w.unserialize(pStream);
        ok |= pStream->read(&time0, sizeof(time0));
        ok |= pStream->read(&time1, sizeof(time1));
        ok |= pStream->read(&lens_radius, sizeof(lens_radius));

        return ok;
    }

private:
    Vector3f origin;
    Vector3f lowerLeftCorner;
    Vector3f horizontal;
    Vector3f vertical;
    Vector3f u, v, w;
    float time0, time1;
    float lens_radius;
};

#endif //PATHTRACER_CAMERA_H