/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "ptQuickSort.h"
#include "ptRNG.h"
#include "ptBVH.h"

BVH::BVH(Hitable** list, int length, float time0, float time1, RNG& rng)
{
    int axis = int(3 * rng.rand());
    quickSort(list, 0, length, axis);
    const auto n = length;
    if (n == 1)
    {
        left = right = list[0];
    }
    else if (n == 2)
    {
        left = list[0];
        right = list[1];
    }
    else
    {
        auto leftNodes = &list[0];
        int leftLen = n/2;
        left = new BVH(leftNodes, leftLen, time0, time1, rng);
        auto rightNodes = &list[n/2];
        int rightLen = length - n / 2;
        right = new BVH(rightNodes, rightLen, time0, time1, rng);
    }
    AABB<float> boxLeft, boxRight;
    left->bounds(time0, time1, boxLeft);
    right->bounds(time0, time1, boxRight);

    m_bbox = join<float>(boxLeft, boxRight);
}

bool BVH::hit(const Rayf &r, float tmin, float tmax, HitRecord &rec) const
{
    if (m_bbox.hit(r, tmin, tmax))
    {
        HitRecord leftRec, rightRec;
        bool hitLeft = left->hit(r, tmin, tmax, leftRec);
        bool hitRight = right->hit(r, tmin, tmax, rightRec);
        if (hitLeft && hitRight)
        {
            if (leftRec.t < rightRec.t)
                rec = leftRec;
            else
                rec = rightRec;
            return true;
        }
        else if (hitLeft)
        {
            rec = leftRec;
            return true;
        }
        else if (hitRight)
        {
            rec = rightRec;
            return true;
        }
        else
            return false;
    }
    return false;
}

bool BVH::bounds(float t0, float t1, AABB<float> &bbox) const
{
    bbox = m_bbox;
    return true;
}

float BVH::pdfValue(const Vector3f& o, const Vector3f& v) const
{
    float weight = 0.5f;
    float sum = weight * left->pdfValue(o, v) + weight * right->pdfValue(o, v);
    return sum;
}

Vector3f BVH::random(const Vector3f& o, RNG& rng) const
{
    if (rng.rand() < 0.5f)
        return left->random(o, rng);
    else
        return right->random(o, rng);
}
