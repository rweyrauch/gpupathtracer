/*
 * CUDA (GPU) Pathtracer based on Peter Shirley's 'Ray Tracing in One Weekend' e-book
 * series.
 *
 * Copyright (C) 2017 by Rick Weyrauch - rpweyrauch@gmail.com
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */
#include <ptBVH.h>
#include "ptHitable.h"
#include "ptHitableList.h"
#include "ptSphere.h"
#include "ptRectangle.h"
#include "ptMedium.h"

COMMON_FUNC Hitable *Hitable::Create(Stream *pStream)
{
    if (pStream == nullptr)
        return nullptr;

    int typeId;
    bool ok = pStream->read(&typeId, sizeof(typeId));
    if (!ok) return nullptr;

    Hitable *hitable = nullptr;

    switch (typeId)
    {
        case ListTypeId:
            hitable = new HitableList();
            break;
        case SphereTypeId:
            hitable = new Sphere();
            break;
        case MovingSphereTypeId:
            hitable = new MovingSphere();
            break;
        case XYRectangleTypeId:
            hitable = new XYRectangle();
            break;
        case XZRectangleTypeId:
            hitable = new XZRectangle();
            break;
        case YZRectangleTypeId:
            hitable = new YZRectangle();
            break;
        case FlipNormalsTypeId:
            hitable = new FlipNormals();
            break;
        case BoxTypeId:
            hitable = new Box();
            break;
        case TranslateTypeId:
            hitable = new Translate();
            break;
        case RotateYTypeId:
            hitable = new RotateY();
            break;
        case MediumTypeId:
            hitable = new ConstantMedium();
            break;
        case BVHTypeId:
            hitable = new BVH();
            break;
        default:
            return nullptr;
    }

    ok = hitable->deserialize(pStream);
    if (!ok)
    {
        delete hitable;
        hitable = nullptr;
    }
    return hitable;
}
