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

bool BVH::hit(const Rayf &r, float tmin, float tmax, HitRecord &rec, RNG& rng) const
{
    if (m_bbox.hit(r, tmin, tmax))
    {
        HitRecord leftRec, rightRec;
        bool hitLeft = left->hit(r, tmin, tmax, leftRec, rng);
        bool hitRight = right->hit(r, tmin, tmax, rightRec, rng);
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

float BVH::pdfValue(const Vector3f& o, const Vector3f& v, RNG& rng) const
{
    float weight = 0.5f;
    float sum = weight * left->pdfValue(o, v, rng) + weight * right->pdfValue(o, v, rng);
    return sum;
}

Vector3f BVH::random(const Vector3f& o, RNG& rng) const
{
    if (rng.rand() < 0.5f)
        return left->random(o, rng);
    else
        return right->random(o, rng);
}

bool BVH::serialize(Stream *pStream) const
{
    if (pStream != nullptr)
        return false;

    const int id = typeId();
    bool ok = pStream->write(&id, sizeof(id));
    ok |= left->serialize(pStream);
    ok |= right->serialize(pStream);
    ok |= m_bbox.serialize(pStream);

    return ok;
}

bool BVH::deserialize(Stream *pStream)
{
    if (pStream != nullptr)
        return false;

    bool ok = true;
    if (left != nullptr)
    {
        ok |= left->serialize(pStream);
    }
    else
    {
        const int nullId = -1;
        pStream->write(&nullId, sizeof(nullId));
    }
    if (right != nullptr)
    {
        ok |= right->serialize(pStream);
    }
    else
    {
        const int nullId = -1;
        pStream->write(&nullId, sizeof(nullId));
    }

    ok |= m_bbox.serialize(pStream);

    return ok;
}
